#include "PythonDroneMobility.h"
#include "inet/common/geometry/common/GeographicCoordinateSystem.h"
#include "inet/common/geometry/common/Quaternion.h"
#include "inet/environment/contract/IGround.h"
#include "inet/mobility/base/MovingMobilityBase.h"
#include "inet/common/geometry/common/Quaternion.h"
#include "inet/mobility/single/VehicleMobility.h"
#include <fstream>
#include "gradys_simulations/protocols/messages/internal/Telemetry_m.h"
#include "gradys_simulations/protocols/messages/internal/PythonMobilityCommand_m.h"

using namespace inet;

namespace gradys_simulations {

using namespace physicalenvironment;

Define_Module(PythonDroneMobility);

void PythonDroneMobility::initialize(int stage) {
    MovingMobilityBase::initialize(stage);
    EV_TRACE << "initializing PythonDroneMobility stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        startTime = par("startTime");
        speed = par("speed");
        heading = 0;

        homeX = par("homeX");
        homeY = par("homeY");
        homeZ = par("homeZ");

        ground = findModuleFromPar<IGround>(par("groundModule"), this);

        telemetryFrequency = par("telemetryFrequency");
        scheduleAt(simTime() + telemetryFrequency, telemetryTimer);

        sendTelemetry();
    }
}

void PythonDroneMobility::setInitialPosition() {
    lastPosition.x = homeX;
    lastPosition.y = homeY;
    lastPosition.z = homeZ;

    lastVelocity.x = speed * cos(M_PI * heading / 180);
    lastVelocity.y = speed * sin(M_PI * heading / 180);
    lastVelocity.z = speed * sin(M_PI * heading / 180);

    if (ground) {
        lastPosition = ground->computeGroundProjection(lastPosition);
        lastVelocity = ground->computeGroundProjection(
                lastPosition + lastVelocity) - lastPosition;
    }

}

void PythonDroneMobility::move() {
    if (simTime() < startTime) {
        return;
    }
    if (instruction != nullptr) {
        switch (instruction->command) {
        case PythonCommand::GOTO_COORDS: {
            fly();
            break;
        }
        case PythonCommand::GOTO_GEO_COORDS: {
            Coord targetPos;
            auto coordinateSystem = findModuleFromPar<
                    IGeographicCoordinateSystem>(par("coordinateSystemModule"),
                    this);
            if (coordinateSystem != nullptr) {
                Coord sceneCoordinate =
                        coordinateSystem->computeSceneCoordinate(
                                GeoCoord(deg(instruction->param1),
                                        deg(instruction->param2),
                                        m(instruction->param3)));
                instruction->param1 = sceneCoordinate.x;
                instruction->param2 = sceneCoordinate.y;
                instruction->param3 = sceneCoordinate.z;
            }
            break;
        }
        case PythonCommand::SET_SPEED: {
            speed = instruction->param1;
            fly();
            break;
        }
        default:
            std::cout << "Something is wrong!" << std::endl;
            exit(1);
        }
    }
}

bool checkIfOvershoot(double origin, double target, double current) {
    double delta = target - origin;
    if (delta > 0) {
        return current > target;
    } else {
        return current < target;
    }
}

void PythonDroneMobility::fly() {
    Coord targetPos;
    targetPos.x = instruction->param1;
    targetPos.y = instruction->param2;
    targetPos.z = instruction->param3;

    Coord previousPosition = lastPosition;

    if (lastPosition.x != targetPos.x || lastPosition.y != targetPos.y) {
        /** 2D move **/
        double dx = targetPos.x - lastPosition.x;
        double dy = targetPos.y - lastPosition.y;

        heading = atan2(dy, dx) / M_PI * 180;
        double timeStep = (simTime() - lastUpdate).dbl();

        Coord tempSpeed = Coord(cos(M_PI * heading / 180),
                sin(M_PI * heading / 180)) * speed;
        Coord tempPosition = lastPosition + tempSpeed * timeStep;

        lastVelocity = tempPosition - lastPosition;
        lastPosition = tempPosition;

        // Checks if we have overshot the waypoint on the x axis
        if (checkIfOvershoot(previousPosition.x, targetPos.x, lastPosition.x)) {
            lastPosition.setX(targetPos.x);
            lastVelocity = lastPosition - previousPosition;
        }
        // Checks if we have overshot the waypoint on the y axis
        if (checkIfOvershoot(previousPosition.y, targetPos.y, lastPosition.y)) {
            lastPosition.setY(targetPos.y);
            lastVelocity = lastPosition - previousPosition;
        }
    }

    if (lastPosition.z != targetPos.z) {
        PythonDroneMobility::climb(targetPos.z);
    }
}

void PythonDroneMobility::climb(double targetZ) {
    if (lastPosition.z != targetZ) {
        double timeStep = (simTime() - lastUpdate).dbl();
        double climbDelta = speed * timeStep;

        // Checks if the climb delta overshoots the target height
        bool climbOvershoots;

        if (lastPosition.z > targetZ) {
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

void PythonDroneMobility::orient() {
    MovingMobilityBase::orient();
}

void PythonDroneMobility::handleMessage(cMessage *message) {
    if (message == telemetryTimer) {
        sendTelemetry();
        scheduleAt(simTime() + telemetryFrequency, telemetryTimer);
    } else {
        PythonMobilityCommand *command =
                dynamic_cast<PythonMobilityCommand*>(message);
        if (command != nullptr) {
            switch (command->getCommandType()) {
            case PythonMobilityCommandType::GOTO_COORD: {
                instruction = new PythonInstruction(PythonCommand::GOTO_COORDS,
                        command->getParam1(), command->getParam2(),
                        command->getParam3(), -1, -1, -1);
                break;
            }
            case PythonMobilityCommandType::GOTO_GEO_COORD: {
                instruction = new PythonInstruction(
                        PythonCommand::GOTO_GEO_COORDS, command->getParam1(),
                        command->getParam2(), command->getParam3(), -1, -1, -1);
                break;
            }
            case PythonMobilityCommandType::SET_SPEED: {
                instruction = new PythonInstruction(
                        PythonCommand::GOTO_GEO_COORDS, command->getParam1(),
                        -1, -1, -1, -1, -1);*.quads[*].mobility.homeY = 0

            }
            delete command;

        } else {
            if (message->isSelfMessage())
                MovingMobilityBase::handleSelfMessage(message);
            else
                throw cRuntimeError(
                        "Mobility modules can only receive self messages");
        }
    }
}

void PythonDroneMobility::sendTelemetry() {
    Enter_Method_Silent("sendTelemetry(%d)", 0);

    Telemetry *message = new Telemetry();

    message->setCurrentX(lastPosition.x);
    message->setCurrentY(lastPosition.y);
    message->setCurrentZ(lastPosition.z);

    for (int i = 0; i < gateSize("protocolGate"); i++) {
        if (gate("protocolGate$o", i)->isConnected()) {
            send(message->dup(), gate("protocolGate$o", i));
        }
    }

    delete message;
}

}
