#include "DroneMobility.h"
#include "inet/common/geometry/common/GeographicCoordinateSystem.h"
#include "inet/common/geometry/common/Quaternion.h"
#include "inet/environment/contract/IGround.h"
#include "inet/mobility/base/MovingMobilityBase.h"
#include "inet/common/geometry/common/GeographicCoordinateSystem.h"
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
}

void DroneMobility::setInitialPosition() {
    VehicleMobility::setInitialPosition();
    lastPosition.z = waypoints[targetPointIndex].timestamp;
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
            if (coordinateSystem != nullptr) {
                Coord sceneCoordinate = coordinateSystem->computeSceneCoordinate(GeoCoord(deg(x), deg(y), m(z)));
                x = sceneCoordinate.x;
                y = sceneCoordinate.y;
                z = sceneCoordinate.z;
            }
            Waypoint instructionWaypoint(x,y,z);

            waypoints.push_back(instructionWaypoint);

            // Set the waypoint index on the instruction
            readInstruction.waypointIndex = waypoints.size() - 1;
        }
        instructions.push_back(readInstruction);
    }
    inputFile.close();
}

void DroneMobility::move() {
    Instruction *currentInstruction = &instructions[currentInstructionIndex];
    switch (currentInstruction->command) {
        case Command::STOP :
            isIdle = true;
            if(idleTime.inUnit(SimTimeUnit::SIMTIME_S) >= currentInstruction->param1) {
                currentInstructionIndex = (currentInstructionIndex + 1) % instructions.size();
                idleTime = SimTime();
                isIdle = false;
            } else {
                idleTime += simTime() - lastUpdate;
            }
            break;
        case Command::GOTO :
            if (isIdle) {
                // TODO: Deal with cases where time isn't an integer
                if(idleTime.inUnit(SimTimeUnit::SIMTIME_S) >= currentInstruction->param1) {
                    currentInstructionIndex = (currentInstructionIndex + 1) % instructions.size();
                    idleTime = SimTime();
                    isIdle = false;
                } else {
                    idleTime += simTime() - lastUpdate;
                }
            }
            else {
                targetPointIndex = currentInstruction->waypointIndex;

                DroneMobility::fly();
            }


            break;
        case Command::JUMP :
            // First time the jump has been reached
            if(currentInstruction->internalCounter == 0) {
                currentInstruction->internalCounter = (int) currentInstruction->param2;
            } else {
                currentInstruction->internalCounter--;

                // Counter has finished
                if (currentInstruction->internalCounter == 0) {
                    currentInstructionIndex++;
                    return;
                }
            }

            // Redirect instruction pointer
            currentInstructionIndex = (int) currentInstruction->param1;

    }
}


void DroneMobility::fly() {
    // Saving height and vertical speed because VehicleMobility::move() resets it
    double lastHeight = lastPosition.z;
    double lastVerticalSpeed = lastVelocity.z;
    VehicleMobility::move();
    lastPosition.z = lastHeight;
    lastVelocity.z = lastVerticalSpeed;
    DroneMobility::climb();

    // Checks if we are close enough to the waypoint and starts to idle
    Waypoint target = waypoints[instructions[currentInstructionIndex].waypointIndex];
    double dx = target.x - lastPosition.x;
    double dy = target.y - lastPosition.y;
    double dz = target.timestamp - lastPosition.z;
    if (dx * dx + dy * dy + dz * dz < waypointProximity * waypointProximity * waypointProximity) {
        isIdle = true;
    }
}

void DroneMobility::climb() {
    // Using the time stamp as Z because it is not used for anything
    double targetZ = waypoints[instructions[currentInstructionIndex].waypointIndex].timestamp;
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

}
