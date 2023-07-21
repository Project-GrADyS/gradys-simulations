#include "DroneMobility.h"
#include "inet/common/geometry/common/GeographicCoordinateSystem.h"
#include "inet/common/geometry/common/Quaternion.h"
#include "inet/environment/contract/IGround.h"
#include "inet/mobility/base/MovingMobilityBase.h"
#include "inet/common/geometry/common/Quaternion.h"
#include "inet/mobility/single/VehicleMobility.h"
#include <fstream>

using namespace inet;

using std::string;
using std::vector;
using std::ifstream;
using std::stod;
using std::stoi;

namespace gradys_simulations {

Define_Module(DroneMobility);

void DroneMobility::initialize(int stage) {
    VehicleMobility::initialize(stage);
    if(stage == 0) {
        verticalSpeed = par("verticalSpeed");
        startTime = par("startTime");
        droneStatus.currentYawSpeed = par("yawSpeed");

        homeLatitude = par("homeLatitude");
        homeLongitude = par("homeLongitude");

        telemetryFrequency = par("telemetryFrequency");
        scheduleAt(simTime() + telemetryFrequency, telemetryTimer);

        sendTelemetry(true);
    }
}

void DroneMobility::setInitialPosition() {
    double x = homeLatitude,y = homeLongitude, z = waypoints[targetPointIndex].timestamp;

    auto coordinateSystem = getModuleFromPar<IGeographicCoordinateSystem>(par("coordinateSystemModule"), this);
    if (coordinateSystem != nullptr) {
        Coord sceneCoordinate = coordinateSystem->computeSceneCoordinate(GeoCoord(deg(x), deg(y), m(0)));
        x = sceneCoordinate.x;
        y = sceneCoordinate.y;
        z = sceneCoordinate.z;
    }
    homeCoords = Coord(x, y, z);
    VehicleMobility::setInitialPosition();
    lastPosition.x = x;
    lastPosition.y = y;
    lastPosition.z = z;
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

    auto coordinateSystem = getModuleFromPar<IGeographicCoordinateSystem>(par("coordinateSystemModule"), this);

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

            createWaypoint(homeLatitude, homeLongitude, z, coordinateSystem);

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
            case MobilityCommandType::RETURN_TO_HOME :
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
            case MobilityCommandType::IDLE_TIME :
            {
                droneStatus.isIdle = true;
                if(simTime() - droneStatus.idleTime >= droneStatus.currentCommandInstance.getParam1()) {
                    droneStatus.currentCommand = -1;
                    droneStatus.isIdle = false;
                    executeCommand();
                }
                break;
            }
            case MobilityCommandType::RECHARGE :
            {
                if(droneStatus.isIdle) {
                    // If the drone is still flying, land it
                    if(abs(lastPosition.z - homeCoords.z) > waypointProximity) {
                        // Removing temporary waypoint that was inserted at back
                        waypoints.pop_back();
                        waypoints.push_back(Waypoint(homeCoords.x, homeCoords.y, homeCoords.z));
                        droneStatus.isIdle = false;
                    }
                    // If the drone is idle and not recharging it just reached home
                    else if(droneStatus.currentActivity != RECHARGING) {
                        // Removing temporary waypoint that was inserted at back
                        waypoints.pop_back();

                        droneStatus.currentActivity = RECHARGING;
                        droneStatus.idleTime = simTime();

                        sendTelemetry();
                    }
                    else {
                        // Recharges forever if the recharge time is less than zero
                       if(droneStatus.currentCommandInstance.getParam1() >= 0 && simTime() - droneStatus.idleTime >= droneStatus.currentCommandInstance.getParam1()) {
                            // Removing idle status from drone so it can proceed
                            droneStatus.isIdle = false;

                            currentInstructionIndex = 0;
                            droneStatus.lastInstructionIndex = -1;
                            droneStatus.currentCommand = -1;

                            // Adjusting isReversed to be coherent to orientation
                            droneStatus.isReversed = false;

                            executeCommand();
                       }
                    }

                }
                else {
                    droneStatus.targetIndex = waypoints.size() - 1;
                    DroneMobility::fly();
                }
                break;

            }
            case MobilityCommandType::FORCE_SHUTDOWN :
            {
                droneStatus.isIdle = true;
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
        Waypoint target = waypoints[targetPointIndex];

        /** 2D move **/
        double dx = target.x - lastPosition.x;
        double dy = target.y - lastPosition.y;

        heading = atan2(dy, dx) / M_PI * 180;
        double timeStep = (simTime() - lastUpdate).dbl();

        Coord tempSpeed = Coord(cos(M_PI * heading / 180), sin(M_PI * heading / 180)) * speed;
        Coord tempPosition = lastPosition + tempSpeed * timeStep;

        lastVelocity = tempPosition - lastPosition;
        lastPosition = tempPosition;

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
            droneStatus.currentActivity = REACHED_EDGE;
        }
    } else {
        currentInstructionIndex++;

        if(currentInstructionIndex >= instructions.size()) {
            currentInstructionIndex = instructions.size() - 1;
            droneStatus.currentActivity = REACHED_EDGE;
        }
    }

    sendTelemetry();
}

//Handles instruction messages and delegates the rest
void DroneMobility::handleMessage(cMessage *message) {
    if(simTime() < startTime) {
            return;
    }

    if(message == telemetryTimer) {
        sendTelemetry();
        scheduleAt(simTime() + telemetryFrequency, telemetryTimer);
    } else {
        MobilityCommand *command = dynamic_cast<MobilityCommand *>(message);
        if(command != nullptr) {
            // Also stops current shutdown comand if it is active and the vehicle
            // receives another command
            if(droneStatus.currentCommand == MobilityCommandType::FORCE_SHUTDOWN && command->getCommandType() == MobilityCommandType::WAKE_UP) {
                droneStatus.currentCommand = -1;
                droneStatus.isIdle = false;

            }

            // Saves current command in queue if a shutdown is coming
            if(command->getCommandType() == MobilityCommandType::FORCE_SHUTDOWN && droneStatus.currentCommand != -1) {
                droneStatus.commandQueue.push(droneStatus.currentCommandInstance.dup());
            }

            droneStatus.commandQueue.push(command);
            // Overrides current queue if it is a shutdown command
            if(command->getCommandType() == MobilityCommandType::FORCE_SHUTDOWN) {
                MobilityCommand *current = droneStatus.commandQueue.front();
                while(!droneStatus.commandQueue.empty() && current != command)
                {
                    // Sends command to back of queue
                    droneStatus.commandQueue.pop();
                    droneStatus.commandQueue.push(current);

                    current = droneStatus.commandQueue.front();
                }
                droneStatus.currentCommand = -1;
            }

            executeCommand();
        } else {
            VehicleMobility::handleMessage(message);
        }
    }
}


void DroneMobility::executeCommand() {
    if(!(droneStatus.commandQueue.size() == 0 || droneStatus.currentCommand != -1)) {
        MobilityCommand *command = droneStatus.commandQueue.front();
        droneStatus.commandQueue.pop();

        droneStatus.currentActivity = FOLLOWING_COMMAND;
        droneStatus.currentCommandInstance = *command;
        switch(command->getCommandType()) {
            case MobilityCommandType::REVERSE:
            {
                droneStatus.isReversed = !droneStatus.isReversed;
                nextInstruction();
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
            case MobilityCommandType::RETURN_TO_HOME:
            {
                Waypoint tempWaypoint = Waypoint(homeCoords.x, homeCoords.y, homeCoords.z);

                currentInstructionIndex = 0;
                droneStatus.lastInstructionIndex = 0;
                droneStatus.currentCommand = RETURN_TO_HOME;

                // Adjusting isReversed to be coherent to orientation
                droneStatus.isReversed = false;

                // Creates and adds a temporary waypoint to the waypoint list
                // to serve as a target
                waypoints.push_back(tempWaypoint);
                droneStatus.isIdle = false;
                break;
            }
            case MobilityCommandType::IDLE_TIME:
            {
                droneStatus.currentCommand = MobilityCommandType::IDLE_TIME;
                droneStatus.isIdle = true;
                droneStatus.idleTime = simTime();

                droneStatus.currentActivity = IDLE;
                break;
            }
            case MobilityCommandType::RECHARGE:
            {
                // Creates and adds two temporary waypoints to the waypoint list
                // to serve as a home target. The first waypoint guarantees the drone
                // doesn't land until it has reached home. The next one is created after
                // it reaches the first
                waypoints.push_back(Waypoint(homeCoords.x, homeCoords.y, lastPosition.z));
                droneStatus.isIdle = false;

                droneStatus.currentCommand = MobilityCommandType::RECHARGE;
                break;

            }
            case MobilityCommandType::FORCE_SHUTDOWN:
            {
                droneStatus.isIdle = true;
                droneStatus.currentCommand = MobilityCommandType::FORCE_SHUTDOWN;

                droneStatus.currentActivity = SHUTDOWN;
                break;
            }
            case MobilityCommandType::WAKE_UP:
            {
                // The command only works on an active FORCE_SHUTDOWN, so it has no meaning in the queue
                executeCommand();
                break;
            }
        }
        delete command;
    }
    sendTelemetry();
}

// Sends telemetry to the communications module
// If sendTour is true attaches tour to the message
void DroneMobility::sendTelemetry(bool sendTour) {
    Enter_Method_Silent("sendTelemetry(%d)", 0);
    Telemetry *message = new Telemetry();
    message->setIsReversed(droneStatus.isReversed);
    message->setNextWaypointID(instructions[currentInstructionIndex].waypointIndex);
    if(droneStatus.lastInstructionIndex >= 0 && droneStatus.lastInstructionIndex < instructions.size()) {
        message->setLastWaypointID(instructions[droneStatus.lastInstructionIndex].waypointIndex);
    }
    message->setCurrentX(lastPosition.x);
    message->setCurrentY(lastPosition.y);
    message->setCurrentZ(lastPosition.z);
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


    for(int i=0;i<gateSize("protocolGate");i++) {
        if(gate("protocolGate$o", i)->isConnected()) {
            send(message->dup(), gate("protocolGate$o", i));
        }
    }

    delete message;
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

DroneMobility::~DroneMobility() {
    cancelAndDelete(telemetryTimer);
}

}
