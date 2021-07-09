#include "DroneMobility.h"
#include "inet/common/geometry/common/GeographicCoordinateSystem.h"
#include "inet/common/geometry/common/Quaternion.h"
#include "inet/environment/contract/IGround.h"
#include "inet/mobility/base/MovingMobilityBase.h"
#include "inet/common/geometry/common/Quaternion.h"
#include "inet/mobility/single/VehicleMobility.h"

#include "Order_m.h"

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
}

void DroneMobility::setInitialPosition() {
    VehicleMobility::setInitialPosition();
    lastPosition.z = waypoints[targetPointIndex].timestamp;
}


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
            DroneMobility::nextInstruction();
            break;
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
    droneStatus.isIdle = false;
    droneStatus.idleTime = 0;
    if(droneStatus.isReversed) {
        currentInstructionIndex = currentInstructionIndex - 1;

        if(currentInstructionIndex < 0) {
            currentInstructionIndex = 0;
            droneStatus.isReversed = false;
        }
    } else {
        currentInstructionIndex++;
    }
}

//Handles instruction messages and delegates the rest
void DroneMobility::handleMessage(cMessage *message) {
    if(simTime() < startTime) {
            return;
    }

    Order *order = dynamic_cast<Order *>(message);
    if(order != nullptr) {
        droneStatus.isReversed = !droneStatus.isReversed;
        if(!droneStatus.isIdle) {
           nextInstruction();
        }
        cancelAndDelete(order);
    } else {
        VehicleMobility::handleMessage(message);
    }
}

}
