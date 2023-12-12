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

        initialLongitude = par("initialLongitude");
        initialLatitude = par("initialLatitude");

        initialX = par("initialX");
        initialY = par("initialY");
        initialZ = par("initialZ");

        ground = findModuleFromPar<IGround>(par("groundModule"), this);

        telemetryFrequency = par("telemetryFrequency");
        scheduleAt(simTime() + telemetryFrequency, telemetryTimer);

        sendTelemetry();
    }
}

void PythonDroneMobility::setInitialPosition() {
    auto coordinateSystem = findModuleFromPar<IGeographicCoordinateSystem>(
            par("coordinateSystemModule"), this);
    if (coordinateSystem != nullptr) {
        Coord sceneCoordinate = coordinateSystem->computeSceneCoordinate(
                GeoCoord(deg(initialLatitude), deg(initialLongitude), m(0)));
        lastPosition.x = sceneCoordinate.x;
        lastPosition.y = sceneCoordinate.y;
        lastPosition.z = sceneCoordinate.z;
    } else {
        lastPosition.x = initialX;
        lastPosition.y = initialY;
        lastPosition.z = initialZ;
    }

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

    fly();
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

                targetPos.x = command->getParam1();
                targetPos.y = command->getParam2();
                targetPos.z = command->getParam3();

                break;
            }
            case PythonMobilityCommandType::GOTO_GEO_COORD: {
                auto coordinateSystem = findModuleFromPar<
                        IGeographicCoordinateSystem>(
                        par("coordinateSystemModule"), this);
                if (coordinateSystem != nullptr) {
                    Coord sceneCoordinate =
                            coordinateSystem->computeSceneCoordinate(
                                    GeoCoord(deg(command->getParam1()),
                                            deg(command->getParam2()),
                                            m(command->getParam3())));

                    targetPos.x = sceneCoordinate.x;
                    targetPos.y = sceneCoordinate.y;
                    targetPos.z = sceneCoordinate.z;
                }

                break;
            }
            case PythonMobilityCommandType::SET_SPEED: {
                speed = command->getParam1();

                break;
            }
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
