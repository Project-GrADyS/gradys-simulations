#include "DroneMobility.h"
#include "inet/common/geometry/common/GeographicCoordinateSystem.h"
#include "inet/common/geometry/common/Quaternion.h"
#include "inet/environment/contract/IGround.h"
#include "inet/mobility/base/MovingMobilityBase.h"
#include "inet/common/geometry/common/Quaternion.h"
#include "inet/mobility/single/VehicleMobility.h"

using namespace inet;

using std::string;
using std::vector;
using std::ifstream;
using std::stod;
using std::stoi;

namespace projeto {

Define_Module(DroneMobility);

void DroneMobility::initialize(int stage) {
    VehicleMobility::initialize(stage);
    verticalSpeed = par("verticalSpeed");
    startTime = par("startTime");
    droneStatus.currentYawSpeed = par("yawSpeed");

    sendTelemetry(true);
}

void DroneMobility::setInitialPosition() {
    VehicleMobility::setInitialPosition();
    lastPosition.z = waypoints[targetPointIndex].timestamp;
}

// Creates and adds a waypoint to the waypoint list following a coordinate system
// Helper function
void DroneMobility::createWaypoint(double x, double y, double z, IGeographicCoordinateSystem *coordinateSystem) {

    if (coordinateSystem != nullptr) {
        Coord sceneCoordinate = coordinateSystem->computeSceneCoordinate(GeoCoord(deg(x), deg(y), m(z)));
        x = sceneCoordinate.x;
        y = sceneCoordinate.y;
        z = sceneCoordinate.z;
    }
    Waypoint instructionWaypoint(x,y,z);
    waypoints.push_back(instructionWaypoint);
}


void DroneMobility::readWaypointsFromFile(const char *fileName) {
    char line[256];
    ifstream inputFile(fileName);

    if(!inputFile.getline(line, 256) || std::strcmp(line, "QGC WPL 110") != 0) {
        EV_ERROR << "Incorrect waypoint file format: " << fileName << " does not have QGC WPL 110 header." << endl;
        return;
    }

    auto coordinateSystem = getModuleFromPar<IGeographicCoordinateSystem>(par("coordinateSystemModule"), this, false);

    // <INDEX> <CURRENT WP> <COORD FRAME> <COMMAND> <PARAM1> <PARAM2> <PARAM3> <PARAM4> <PARAM5/X/LATITUDE> <PARAM6/Y/LONGITUDE> <PARAM7/Z/ALTITUDE> <AUTOCONTINUE>
    while (inputFile.getline(line, 256)) {
        cStringTokenizer tokenizer(line, "\t");
        vector<string> lineVector = tokenizer.asVector();

        if (lineVector.size() != 12) {
            EV_ERROR << "Waypoint file format error." << endl;
            instructions.clear();
            waypoints.clear();
            break;
        }

        // Translates instruction to struct
        Instruction readInstruction(
            static_cast<Command>(stoi(lineVector[3])),
            stod(lineVector[4]),
            stod(lineVector[5]),
            stod(lineVector[6]),
            stod(lineVector[7]),
            lineVector[11] == "1"
        );

        // If the command requires a waypoint, create one
        if (readInstruction.command == Command::GOTO) {
            double x = stod(lineVector[8]);
            double y = stod(lineVector[9]);
            double z = stod(lineVector[10]);

            createWaypoint(x, y, z, coordinateSystem);

            // Set the waypoint index on the instruction
            readInstruction.waypointIndex = waypoints.size() - 1;
        }
        else if (readInstruction.command == Command::TAKEOFF) {
            double z = stod(lineVector[10]);

            createWaypoint(0, 0, z, nullptr);

            // Set the waypoint index on the instruction
            readInstruction.waypointIndex = waypoints.size() - 1;
        }
        else if(readInstruction.command == Command::RETURN_LAUNCH) {
                if(waypoints.size() > 0)
                {
                //  Navigates to the first waypoint maintaining height and them lands
                Instruction auxiliaryInstruction(Command::GOTO, 0, 0, 0, 0, true);

                createWaypoint(waypoints[0].x, waypoints[0].y, waypoints.back().timestamp, nullptr);
                auxiliaryInstruction.waypointIndex = waypoints.size() - 1;
                instructions.push_back(auxiliaryInstruction);
            }
        }

        instructions.push_back(readInstruction);
    }

    inputFile.close();
}

void DroneMobility::move() {
    if(simTime() < startTime) {
        return;
    }

    if (currentInstructionIndex >= instructions.size()) {
        return;
    }

    Instruction *currentInstruction = &instructions[currentInstructionIndex];

    if(droneStatus.currentCommand != -1) {
        switch(droneStatus.currentCommand) {
            case MobilityCommandType::GOTO_WAYPOINT :
            {
                if(droneStatus.isIdle)
                {
                    if(droneStatus.targetIndex < droneStatus.gotoWaypointTarget) {
                        droneStatus.targetIndex++;
                    } else if(droneStatus.targetIndex > droneStatus.gotoWaypointTarget) {
                        droneStatus.targetIndex--;
                    }
                    // Setting the correct instruction so the drone proceeds normally
                    int index = instructionIndexFromWaypoint(droneStatus.gotoWaypointTarget);
                    droneStatus.lastInstructionIndex = currentInstructionIndex;
                    currentInstructionIndex = index;

                    droneStatus.isIdle = false;
                    DroneMobility::fly();

                    // If the drone has reached it's destination reset and execute the next command
                    if(droneStatus.targetIndex == droneStatus.gotoWaypointTarget && droneStatus.isIdle) {
                        droneStatus.currentCommand = -1;
                        droneStatus.gotoWaypointTarget = -1;
                        executeCommand();
                    }
                } else {
                    DroneMobility::fly();
                }
                break;
            }
            case MobilityCommandType::GOTO_COORDS :
            {
                if(droneStatus.isIdle) {
                    droneStatus.currentCommand = -1;

                    // Removing temporary waypoint that was inserted at back
                    waypoints.pop_back();

                    // Removing idle status from drone so it can proceed
                    droneStatus.isIdle = false;

                    executeCommand();
                }
                else {
                    droneStatus.targetIndex = waypoints.size() - 1;
                    DroneMobility::fly();
                }
                break;
            }
        }
    }
    else {
        droneStatus.currentActivity = NAVIGATING;
        switch (currentInstruction->command) {
            case Command::STOP :
            {
                droneStatus.isIdle = true;
                if(droneStatus.idleTime >= currentInstruction->param1) {
                    DroneMobility::nextInstruction();
                    droneStatus.idleTime = SimTime();
                    droneStatus.isIdle = false;
                } else {
                    droneStatus.idleTime += simTime() - lastUpdate;
                }
                break;
            }
            case Command::GOTO :
            {
                if (droneStatus.isIdle) {
                    if(droneStatus.idleTime >= currentInstruction->param1) {
                        DroneMobility::nextInstruction();
                        droneStatus.idleTime = SimTime();
                        droneStatus.isIdle = false;
                    } else {
                        droneStatus.idleTime += simTime() - lastUpdate;
                    }
                }
                else {
                    droneStatus.targetIndex = currentInstruction->waypointIndex;
                    DroneMobility::fly();
                }
                break;
            }
            case Command::JUMP :
            {
                // Loop of size -1 repeats forever
                if(currentInstruction->param2 != -1) {

                    // First time the jump has been reached
                    if(currentInstruction->internalCounter == 0) {
                        currentInstruction->internalCounter = (int) currentInstruction->param2;
                    }
                    else {
                        currentInstruction->internalCounter--;

                        // Counter has finished
                        if (currentInstruction->internalCounter == 0) {
                            DroneMobility::nextInstruction();
                            return;
                        }
                    }
                }

                // Redirect instruction pointer
                currentInstructionIndex = (int) currentInstruction->param1;
                break;
            }

            case Command::TAKEOFF :
            {
                double targetHeight = waypoints[currentInstruction->waypointIndex].timestamp;
                DroneMobility::climb(targetHeight);

                if(targetHeight - lastPosition.z  < waypointProximity) {
                    DroneMobility::nextInstruction();
                }
                break;
            }

            case Command::RETURN_LAUNCH :
            {
                DroneMobility::climb(0);
                if(lastPosition.z < waypointProximity) {
                    DroneMobility::nextInstruction();
                }
                break;
            }

            case Command::YAW :
            {
                // 0 = Absolute; 1 = Relative;
                if(currentInstruction->param4 == 0) {
                    droneStatus.currentYaw = rad(deg(currentInstruction->param1)).get();
                }
                else {
                    droneStatus.currentYaw = rad(deg(currentInstruction->param1)).get() + lastOrientation.getRotationAngle();
                }

                // If the yawSpeed is 0, reset to default
                if(currentInstruction->param2 == 0) {
                    droneStatus.currentYawSpeed = par("yawSpeed");
                }
                else {
                    droneStatus.currentYawSpeed = rad(deg(currentInstruction->param2)).get();
                }

                DroneMobility::nextInstruction();
                break;
            }
            case Command::REVERSE :
            {
                droneStatus.isReversed = true;
                droneStatus.currentActivity = REACHED_EDGE;
                DroneMobility::nextInstruction();
                break;
            }
        }
    }
}

bool checkIfOvershoots(double origin, double target, double current) {
    double delta = target - origin;

    if(delta > 0) {
        return current > target;
    } else {
        return current < target;
    }
}

void DroneMobility::orient() {
    if(simTime() < startTime) {
            return;
    }

    if(droneStatus.currentYaw >= 0) {
        double lastYaw = lastOrientation.getRotationAngle();
        if(lastYaw != droneStatus.currentYaw) {
            double yawIncrement = droneStatus.currentYawSpeed * (simTime() - lastUpdate).dbl();

            if(lastYaw > droneStatus.currentYaw) {
                yawIncrement *= -1;
            }

            if(yawIncrement > 0 && lastYaw + yawIncrement > droneStatus.currentYaw) {
                yawIncrement = droneStatus.currentYaw - lastYaw;
            }
            else if(yawIncrement < 0 && lastYaw + yawIncrement < droneStatus.currentYaw) {
                yawIncrement = droneStatus.currentYaw - lastYaw;
            }

            EulerAngles angles = lastOrientation.toEulerAngles();
            angles.setAlpha(angles.getAlpha() + rad(yawIncrement));

            lastOrientation = Quaternion(angles);
        }
    } else {
        Coord target = Coord(waypoints[targetPointIndex].x, waypoints[targetPointIndex].y, 0);

        Coord currentLocation = lastPosition;
        currentLocation.setZ(0);

        // Doesn't orient if the horizontal distance to the target is below tolerance
        if(target.sqrdist(currentLocation) > waypointProximity * waypointProximity) {
            VehicleMobility::orient();
        }
    }
}

void DroneMobility::fly() {
    Waypoint target = waypoints[droneStatus.targetIndex];
    Coord coordTarget = Coord(target.x, target.y, target.timestamp);

    // Saving height and vertical speed because VehicleMobility::move() resets it
    Coord previousPosition = lastPosition;
    Coord previousVelocity = lastVelocity;

    if(lastPosition.x != coordTarget.x || lastPosition.y != coordTarget.y) {
        // Setting the waypoint index for VehicleMobility::move()
        targetPointIndex = droneStatus.targetIndex;
        VehicleMobility::move();

        // Checks if we have overshot the waypoint on the x axis
        if (checkIfOvershoots(previousPosition.x, target.x, lastPosition.x)) {
            lastPosition.setX(coordTarget.x);
            lastVelocity = lastPosition - previousPosition;
        }
        // Checks if we have overshot the waypoint on the y axis
        if (checkIfOvershoots(previousPosition.y, target.y, lastPosition.y)) {
            lastPosition.setY(coordTarget.y);
            lastVelocity = lastPosition - previousPosition;
        }
    }

    // Restoring height, reset by VehicleMobility::move()
    lastPosition.z = previousPosition.z;
    lastVelocity.z = previousVelocity.z;

    if(lastPosition.z != coordTarget.z) {
        DroneMobility::climb(target.timestamp);
    }

    // Check if we are close enough to the waypoint to resume to the next
    if (lastPosition.sqrdist(coordTarget) < waypointProximity * waypointProximity) {
        droneStatus.isIdle = true;
    }
}

void DroneMobility::climb(double targetZ) {
    if(lastPosition.z != targetZ) {
        double timeStep = (simTime() - lastUpdate).dbl();
        double climbDelta =  verticalSpeed * timeStep;

        // Checks if the climb delta overshoots the target height
        bool climbOvershoots;

        if(lastPosition.z > targetZ) {
            // If the target position is below the current position, descend
            climbDelta = climbDelta * -1;
            climbOvershoots = (lastPosition.z + climbDelta) < targetZ;
        } else {
            climbOvershoots = (lastPosition.z + climbDelta) > targetZ;
        }

        climbDelta = climbOvershoots ? targetZ - lastPosition.z : climbDelta;

        lastPosition.z += climbDelta;
        lastVelocity.z = climbDelta;
    }
}

void DroneMobility::nextInstruction() {
    droneStatus.lastInstructionIndex = currentInstructionIndex;

    droneStatus.isIdle = false;
    droneStatus.idleTime = 0;
    if(droneStatus.isReversed) {
        currentInstructionIndex = currentInstructionIndex - 1;

        if(currentInstructionIndex < 0) {
            currentInstructionIndex = 0;
            droneStatus.isReversed = false;
            droneStatus.currentActivity = REACHED_EDGE;
        }
    } else {
        currentInstructionIndex++;
    }


    sendTelemetry();
}

//Handles instruction messages and delegates the rest
void DroneMobility::handleMessage(cMessage *message) {
    if(simTime() < startTime) {
            return;
    }

    MobilityCommand *command = dynamic_cast<MobilityCommand *>(message);
    if(command != nullptr) {
        droneStatus.commandQueue.push(command);
        executeCommand();
    } else {
        VehicleMobility::handleMessage(message);
    }
}


void DroneMobility::executeCommand() {
    if(!(droneStatus.commandQueue.size() == 0 || droneStatus.currentCommand != -1)) {
        MobilityCommand *command = droneStatus.commandQueue.front();
        droneStatus.commandQueue.pop();
        switch(command->getCommandType()) {
            case MobilityCommandType::REVERSE:
            {
                droneStatus.isReversed = !droneStatus.isReversed;

                // Reverses current instructions
                int temp = currentInstructionIndex;
                currentInstructionIndex = droneStatus.lastInstructionIndex;
                droneStatus.lastInstructionIndex = temp;

                break;
            }
            case MobilityCommandType::GOTO_WAYPOINT:
            {
                // Setting command
                droneStatus.currentCommand = GOTO_WAYPOINT;
                droneStatus.gotoWaypointTarget = command->getParam1();

                // Idles and inverts if the drone is travelling in the oposite direction of the
                // waypoint
                if(droneStatus.targetIndex > droneStatus.gotoWaypointTarget && !droneStatus.isReversed) {
                    droneStatus.isIdle = true;
                } else if(droneStatus.targetIndex < droneStatus.gotoWaypointTarget && droneStatus.isReversed) {
                    droneStatus.isIdle = true;
                }

                break;
            }
            case MobilityCommandType::GOTO_COORDS:
            {
                Waypoint tempWaypoint = Waypoint(
                        command->getParam1(),
                        command->getParam2(),
                        command->getParam3());

                // Sets the target to be followed after the drone has reached it's destination
                int nextIndex = instructionIndexFromWaypoint(command->getParam4());
                currentInstructionIndex = nextIndex;

                // Sets the waypoint the dron used to get to the destination
                int lastIndex = instructionIndexFromWaypoint(command->getParam5());
                droneStatus.lastInstructionIndex = lastIndex;
                droneStatus.currentCommand = GOTO_COORDS;

                // Adjusting isReversed to be coherent to orientation
                droneStatus.isReversed = nextIndex < lastIndex;

                // Creates and adds a temporary waypoint to the waypoint list
                // to serve as a target
                waypoints.push_back(tempWaypoint);
                droneStatus.isIdle = false;
                break;
            }
            droneStatus.currentActivity = FOLLOWING_COMMAND;
        }
        delete command;
    }
    sendTelemetry();
}

// Sends telemetry to the communications module
// If sendTour is true attaches tour to the message
void DroneMobility::sendTelemetry(bool sendTour) {
    Enter_Method_Silent("sendTelemetry(%d)", 0);
    Telemetry *message = new Telemetry("Telemetry", 0);
    message->setIsReversed(droneStatus.isReversed);
    message->setNextWaypointID(instructions[currentInstructionIndex].waypointIndex);
    if(droneStatus.lastInstructionIndex >= 0 && droneStatus.lastInstructionIndex < instructions.size()) {
        message->setLastWaypointID(instructions[droneStatus.lastInstructionIndex].waypointIndex);
    }
    message->setCurrentCommand(droneStatus.currentCommand);
    message->setDroneActivity(droneStatus.currentActivity);

    if(sendTour) {
        std::vector<Coord> *coordArray = new std::vector<Coord>;
        for(Waypoint waypoint : waypoints) {
            Coord coord = Coord(waypoint.x, waypoint.y, waypoint.timestamp);
            coordArray->push_back(coord);
        }
        message->addPar("tourCoords");
        message->par("tourCoords").setPointerValue(coordArray);
    }

    send(message, gate("commGate$o"));
}


int DroneMobility::instructionIndexFromWaypoint(int waypointIndex) {
    // Setting the correct instruction equivalent to the specified next waypoint
    int index;
    for(index=0;index<instructions.size();index++) {
       if(instructions[index].waypointIndex == waypointIndex) {
           break;
       }
    }
    return index;
}

}
