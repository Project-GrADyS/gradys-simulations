#include "PythonDroneMobility.h"
#include "inet/common/geometry/common/GeographicCoordinateSystem.h"
#include "inet/common/geometry/common/Quaternion.h"
#include "inet/environment/contract/IGround.h"
#include "inet/mobility/base/MovingMobilityBase.h"
#include "inet/common/geometry/common/Quaternion.h"
#include "inet/mobility/single/VehicleMobility.h"
#include <fstream>
#include "gradys_simulations/protocols/messages/internal/PythonMobilityCommand_m.h"

using namespace inet;

namespace gradys_simulations {

using namespace physicalenvironment;

Define_Module(PythonDroneMobility);

void PythonDroneMobility::initialize(int stage) {
    MovingMobilityBase::initialize(stage);
    EV_TRACE << "initializing PythonDroneMobility stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        speed = par("speed");
        heading = 0;
        ground = findModuleFromPar<IGround>(par("groundModule"), this);
        startTime = par("startTime");
        telemetryFrequency = par("telemetryFrequency");
        scheduleAt(simTime() + telemetryFrequency, telemetryTimer);
    }
}

void PythonDroneMobility::setInitialPosition() {
    auto coordinateSystem = findModuleFromPar<IGeographicCoordinateSystem>(
            par("coordinateSystemModule"), this);
    if (coordinateSystem == nullptr) {
        lastPosition.x = instruction->param1;
        lastPosition.y = instruction->param2;
        lastPosition.z = instruction->param3;
    } else {
        Coord sceneCoordinate = coordinateSystem->computeSceneCoordinate(
                GeoCoord(deg(instruction->param1), deg(instruction->param2),
                        m(instruction->param3)));
        lastPosition.x = sceneCoordinate.x;
        lastPosition.y = sceneCoordinate.y;
        lastPosition.z = sceneCoordinate.z;
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

    switch (instruction->command) {
    case PythonCommand::SET_START: {
        setInitialPosition();
    }
    case PythonCommand::GOTO_COORDS:
    case PythonCommand::GOTO_GEO_COORDS: {
        fly();
    }
    case PythonCommand::SET_SPEED: {

    }
    }
}

void PythonDroneMobility::fly() {
    Coord targetPos;
    auto coordinateSystem = findModuleFromPar<IGeographicCoordinateSystem>(
            par("coordinateSystemModule"), this);
    if (coordinateSystem == nullptr) {
        targetPos.x = instruction->param1;
        targetPos.y = instruction->param2;
        targetPos.z = instruction->param3;
    } else {
        Coord sceneCoordinate = coordinateSystem->computeSceneCoordinate(
                GeoCoord(deg(instruction->param1), deg(instruction->param2),
                        m(instruction->param3)));
        targetPos.x = sceneCoordinate.x;
        targetPos.y = sceneCoordinate.y;
        targetPos.z = sceneCoordinate.z;
    }

    if(lastPosition.x != targetPos.x || lastPosition.y != targetPos.y) {

        double dx = targetPos.x - lastPosition.x;
        double dy = targetPos.y - lastPosition.y;
        double targetDirection = atan2(dy, dx) / M_PI * 180;
        double diff = targetDirection - heading;
        while (diff < -180)
            diff += 360;
        while (diff > 180)
            diff -= 360;
        angularSpeed = diff * 5;
        double timeStep = (simTime() - lastUpdate).dbl();
        heading += angularSpeed * timeStep;

        Coord tempSpeed = Coord(cos(M_PI * heading / 180),
                sin(M_PI * heading / 180)) * speed;
        Coord tempPosition = lastPosition + tempSpeed * timeStep;

        if (ground)
            tempPosition = ground->computeGroundProjection(tempPosition);

        lastVelocity = tempPosition - lastPosition;
        lastPosition = tempPosition;
    }
}

void PythonDroneMobility::orient() {
    if (ground) {
        Coord groundNormal = ground->computeGroundNormal(lastPosition);

        // this will make the wheels follow the ground
        Quaternion quat = Quaternion::rotationFromTo(Coord(0, 0, 1),
                groundNormal);

        Coord groundTangent = groundNormal % lastVelocity;
        groundTangent.normalize();
        Coord direction = groundTangent % groundNormal;
        direction.normalize(); // this is lastSpeed, normalized and adjusted to be perpendicular to groundNormal

        // our model looks in this direction if we only rotate the Z axis to match the ground normal
        Coord groundX = quat.rotate(Coord(1, 0, 0));

        double dp = groundX * direction;

        double angle;

        if (((groundX % direction) * groundNormal) > 0)
            angle = std::acos(dp);
        else
            // correcting for the case where the angle should be over 90 degrees (or under -90):
            angle = 2 * M_PI - std::acos(dp);

        // and finally rotating around the now-ground-orthogonal local Z
        quat *= Quaternion(Coord(0, 0, 1), angle);

        lastOrientation = quat;
    } else
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
            case PythonMobilityCommandType::SET_START: {
                instruction = new PythonInstruction(PythonCommand::SET_START,
                        command->getParam1(), command->getParam2(),
                        command->getParam3(), -1, -1, -1);
                break;
            }
            case PythonMobilityCommandType::GOTO_COORDS: {
                instruction = new PythonInstruction(PythonCommand::GOTO_COORDS,
                        command->getParam1(), command->getParam2(),
                        command->getParam3(), -1, -1, -1);
                break;
            }
            case PythonMobilityCommandType::GOTO_GEO_COORDS: {
                instruction = new PythonInstruction(
                        PythonCommand::GOTO_GEO_COORDS, command->getParam1(),
                        command->getParam2(), command->getParam3(), -1, -1, -1);
                break;
            }
            case PythonMobilityCommandType::SET_SPEED: {
                instruction = new PythonInstruction(
                        PythonCommand::GOTO_GEO_COORDS, command->getParam1(),
                        -1, -1, -1, -1, -1);
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

}
