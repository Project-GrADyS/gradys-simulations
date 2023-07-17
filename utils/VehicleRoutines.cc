/*
 * VehicleRoutines.cc
 *
 *  Created on: 12 de mar de 2022
 *      Author: thlam
 */

#include "VehicleRoutines.h"
#include <omnetpp.h>
#include "mavlink/ardupilotmega/mavlink.h"
#include "utils/TelemetryConditions.h"

namespace projeto {
namespace VehicleRoutines {

std::vector<std::shared_ptr<Instruction>> armTakeoffCopter(uint8_t senderSystem, uint8_t senderComponent, float altitude, uint8_t targetSystem, uint8_t targetComponent, omnetpp::simtime_t timeout,
        int retries)
{
    std::vector<std::shared_ptr<Instruction>> instructions;

    mavlink_command_long_t cmd = {};
    mavlink_message_t msg = {};

    cmd.command = MAV_CMD_DO_SET_MODE;
    cmd.confirmation = 0;
    cmd.param1 = MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;
    cmd.param2 = COPTER_MODE_GUIDED;
    cmd.target_component = targetComponent;
    cmd.target_system = targetSystem;

    mavlink_msg_command_long_encode(senderSystem, senderComponent, &msg, &cmd);
    instructions.push_back(std::make_shared<Instruction>(msg, TelemetryConditions::getCheckPreArm(targetSystem), timeout, retries, "Setting mode to GUIDED"));

    cmd = {};
    cmd.command = MAV_CMD_COMPONENT_ARM_DISARM;
    cmd.confirmation = 0;
    cmd.param1 = 1;
    cmd.target_component = targetComponent;
    cmd.target_system = targetSystem;

    mavlink_msg_command_long_encode(senderSystem, senderComponent, &msg, &cmd);
    instructions.push_back(std::make_shared<Instruction>(msg, TelemetryConditions::getCheckArm(targetSystem), timeout, retries, "Arming vehicle"));

    cmd = {};
    cmd.command = MAV_CMD_NAV_TAKEOFF;
    cmd.confirmation = 0;
    cmd.param7 = altitude;
    cmd.target_component = targetComponent;
    cmd.target_system = targetSystem;

    mavlink_msg_command_long_encode(senderSystem, senderComponent, &msg, &cmd);
    instructions.push_back(std::make_shared<Instruction>(msg, TelemetryConditions::getCheckAltitude(altitude, 3, targetSystem), timeout, retries, "Sending TAKEOFF command"));

    return instructions;
}

std::vector<std::shared_ptr<Instruction>> armTakeoffPlane(uint8_t senderSystem, uint8_t senderComponent, float altitude, uint8_t targetSystem, uint8_t targetComponent, omnetpp::simtime_t timeout,
        int retries)
{
    std::vector<std::shared_ptr<Instruction>> instructions;

    mavlink_command_long_t cmd = {};
    mavlink_message_t msg = {};

    mavlink_param_set_t set_altitude { altitude, targetSystem, targetComponent, "TKOFF_ALT", MAV_PARAM_TYPE_REAL32 };
    mavlink_msg_param_set_encode(senderSystem, senderComponent, &msg, &set_altitude);
    instructions.push_back(
            std::make_shared<Instruction>(msg, TelemetryConditions::getCheckParamValue("TKOFF_ALT", altitude, targetSystem), timeout, retries, "Setting takeoff altitude"));

    for (std::shared_ptr<Instruction> instruction : setMode(senderSystem, senderComponent, PLANE, TAKEOFF, targetSystem, targetComponent, timeout, retries)) {
        instructions.push_back(instruction);
    }

    cmd = {};
    cmd.command = MAV_CMD_COMPONENT_ARM_DISARM;
    cmd.confirmation = 0;
    cmd.param1 = 1;
    cmd.target_component = targetComponent;
    cmd.target_system = targetSystem;
    mavlink_msg_command_long_encode(senderSystem, senderComponent, &msg, &cmd);
    instructions.push_back(std::make_shared<Instruction>(msg, TelemetryConditions::getCheckAltitude(altitude, 3, targetSystem), timeout, retries, "Arming vehicle"));

    return instructions;
}

std::vector<std::shared_ptr<Instruction>> armTakeoffRover(uint8_t senderSystem, uint8_t senderComponent, uint8_t targetSystem, uint8_t targetComponent, omnetpp::simtime_t timeout, int retries)
{
    std::vector<std::shared_ptr<Instruction>> instructions;

    mavlink_command_long_t cmd = {};
    mavlink_message_t msg = {};

    cmd.command = MAV_CMD_DO_SET_MODE;
    cmd.confirmation = 0;
    cmd.param1 = MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;
    cmd.param2 = COPTER_MODE_GUIDED;
    cmd.target_component = targetComponent;
    cmd.target_system = targetSystem;

    mavlink_msg_command_long_encode(senderSystem, senderComponent, &msg, &cmd);
    instructions.push_back(std::make_shared<Instruction>(msg, TelemetryConditions::getCheckPreArm(targetSystem), timeout, retries, "Setting mode to GUIDED"));

    cmd = {};
    cmd.command = MAV_CMD_COMPONENT_ARM_DISARM;
    cmd.confirmation = 0;
    cmd.param1 = 1;
    cmd.target_component = targetComponent;
    cmd.target_system = targetSystem;

    mavlink_msg_command_long_encode(senderSystem, senderComponent, &msg, &cmd);
    instructions.push_back(std::make_shared<Instruction>(msg, TelemetryConditions::getCheckArm(targetSystem), timeout, retries, "Arming vehicle"));

    return instructions;
}


std::vector<std::shared_ptr<Instruction>> armTakeoff(uint8_t senderSystem, uint8_t senderComponent, VehicleType type, float altitude, uint8_t targetSystem, uint8_t targetComponent,
        omnetpp::simtime_t timeout, int retries)
{
    switch (type) {
        case COPTER:
            return armTakeoffCopter(senderSystem, senderComponent, altitude, targetSystem, targetComponent, timeout, retries);
        case PLANE:
            return armTakeoffPlane(senderSystem, senderComponent, altitude, targetSystem, targetComponent, timeout, retries);
        case ROVER:
            return armTakeoffRover(senderSystem, senderComponent, targetSystem, targetComponent, timeout, retries);
    }
    return {};
}

std::vector<std::shared_ptr<Instruction>> setModeCopter(uint8_t senderSystem, uint8_t senderComponent, Mode mode, uint8_t targetSystem, uint8_t targetComponent, omnetpp::simtime_t timeout,
        int retries)
{
    std::vector<std::shared_ptr<Instruction>> instructions;
    mavlink_command_long_t cmd = {};
    mavlink_message_t msg = {};
    switch (mode) {
        case GUIDED:
            cmd.command = MAV_CMD_DO_SET_MODE;
            cmd.confirmation = 0;
            cmd.param1 = MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;
            cmd.param2 = COPTER_MODE_GUIDED;
            cmd.target_component = targetComponent;
            cmd.target_system = targetSystem;

            mavlink_msg_command_long_encode(senderSystem, senderComponent, &msg, &cmd);
            instructions.push_back(
                    std::make_shared<Instruction>(msg,
                            TelemetryConditions::getCheckMode(GUIDED, COPTER, targetSystem), timeout, retries, "Seting mode to GUIDED"));
            break;
        case AUTO:
            cmd.command = MAV_CMD_DO_SET_MODE;
            cmd.confirmation = 0;
            cmd.param1 = MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;
            cmd.param2 = COPTER_MODE_AUTO;
            cmd.target_component = targetComponent;
            cmd.target_system = targetSystem;
            mavlink_msg_command_long_encode(senderSystem, senderComponent, &msg, &cmd);
            instructions.push_back(
                    std::make_shared<Instruction>(msg,
                            TelemetryConditions::getCheckMode(AUTO, COPTER, targetSystem), timeout, retries, "Setting mode to AUTO"));
            break;
        case TAKEOFF:
            break;
    }
    return instructions;
}

std::vector<std::shared_ptr<Instruction>> setModePlane(uint8_t senderSystem, uint8_t senderComponent, Mode mode, uint8_t targetSystem, uint8_t targetComponent, omnetpp::simtime_t timeout,
        int retries)
{
    std::vector<std::shared_ptr<Instruction>> instructions;
    mavlink_command_long_t cmd = {};
    mavlink_message_t msg = {};
    switch (mode) {
        case GUIDED:
            cmd.command = MAV_CMD_DO_SET_MODE;
            cmd.confirmation = 0;
            cmd.param1 = MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;
            cmd.param2 = PLANE_MODE_GUIDED;
            cmd.target_component = targetComponent;
            cmd.target_system = targetSystem;

            mavlink_msg_command_long_encode(senderSystem, senderComponent, &msg, &cmd);
            instructions.push_back(
                    std::make_shared<Instruction>(msg,
                            TelemetryConditions::getCheckMode(GUIDED, PLANE, targetSystem), timeout, retries, "Setting mode to GUIDED"));
            break;
        case AUTO:
            cmd.command = MAV_CMD_DO_SET_MODE;
            cmd.confirmation = 0;
            cmd.param1 = MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;
            cmd.param2 = PLANE_MODE_AUTO;
            cmd.target_component = targetComponent;
            cmd.target_system = targetSystem;
            mavlink_msg_command_long_encode(senderSystem, senderComponent, &msg, &cmd);
            instructions.push_back(
                    std::make_shared<Instruction>(msg,
                            TelemetryConditions::getCheckMode(AUTO, PLANE, targetSystem), timeout, retries, "Setting mode to AUTO"));
            break;
        case TAKEOFF:
            cmd.command = MAV_CMD_DO_SET_MODE;
            cmd.confirmation = 0;
            cmd.param1 = MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;
            cmd.param2 = PLANE_MODE_TAKEOFF;
            cmd.target_component = targetComponent;
            cmd.target_system = targetSystem;
            mavlink_msg_command_long_encode(senderSystem, senderComponent, &msg, &cmd);
            instructions.push_back(
                    std::make_shared<Instruction>(msg,
                            TelemetryConditions::getCheckMode(TAKEOFF, PLANE, targetSystem), timeout, retries, "Setting mode to TAKEOFF"));
            break;
    }
    return instructions;
}

std::vector<std::shared_ptr<Instruction>> setModeRover(uint8_t senderSystem, uint8_t senderComponent, Mode mode, uint8_t targetSystem, uint8_t targetComponent, omnetpp::simtime_t timeout,
        int retries)
{
    std::vector<std::shared_ptr<Instruction>> instructions;
    mavlink_command_long_t cmd = {};
    mavlink_message_t msg = {};
    switch (mode) {
        case GUIDED:
            cmd.command = MAV_CMD_DO_SET_MODE;
            cmd.confirmation = 0;
            cmd.param1 = MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;
            cmd.param2 = ROVER_MODE_GUIDED;
            cmd.target_component = targetComponent;
            cmd.target_system = targetSystem;

            mavlink_msg_command_long_encode(senderSystem, senderComponent, &msg, &cmd);
            instructions.push_back(
                    std::make_shared<Instruction>(msg,
                            TelemetryConditions::getCheckMode(GUIDED, ROVER, targetSystem), timeout, retries, "Setting mode to GUIDED"));
            break;
        case AUTO:
            cmd.command = MAV_CMD_DO_SET_MODE;
            cmd.confirmation = 0;
            cmd.param1 = MAV_MODE_FLAG_CUSTOM_MODE_ENABLED;
            cmd.param2 = ROVER_MODE_AUTO;
            cmd.target_component = targetComponent;
            cmd.target_system = targetSystem;
            mavlink_msg_command_long_encode(senderSystem, senderComponent, &msg, &cmd);
            instructions.push_back(
                    std::make_shared<Instruction>(msg,
                            TelemetryConditions::getCheckMode(AUTO, ROVER, targetSystem), timeout, retries, "Setting mode to AUTO"));
            break;
    }
    return instructions;
}


std::vector<std::shared_ptr<Instruction>> setMode(uint8_t senderSystem, uint8_t senderComponent, VehicleType type, Mode mode, uint8_t targetSystem, uint8_t targetComponent,
        omnetpp::simtime_t timeout, int retries)
{
    switch (type) {
        case COPTER:
            return setModeCopter(senderSystem, senderComponent, mode, targetSystem, targetComponent, timeout, retries);
        case PLANE:
            return setModePlane(senderSystem, senderComponent, mode, targetSystem, targetComponent, timeout, retries);
        case ROVER:
            return setModeRover(senderSystem, senderComponent, mode, targetSystem, targetComponent, timeout, retries);
    }
    return {};
}

// https://ardupilot.org/dev/docs/copter-commands-in-guided-mode.html
std::vector<std::shared_ptr<Instruction>> guidedGotoCopter(double latitude, double longitude, float altitude, double tolerance,
        inet::IGeographicCoordinateSystem *coordinateSystem, uint8_t targetSystem, uint8_t targetComponent, omnetpp::simtime_t timeout, int retries)
{
    std::vector<std::shared_ptr<Instruction>> instructions;

    mavlink_set_position_target_global_int_t position_command = {};
    position_command.lat_int = latitude * (1e7);
    position_command.lon_int = longitude * (1e7);
    position_command.alt = altitude;
    position_command.type_mask = POSITION_TARGET_TYPEMASK_VX_IGNORE | POSITION_TARGET_TYPEMASK_VY_IGNORE | POSITION_TARGET_TYPEMASK_VZ_IGNORE
            | POSITION_TARGET_TYPEMASK_AX_IGNORE | POSITION_TARGET_TYPEMASK_AY_IGNORE | POSITION_TARGET_TYPEMASK_AZ_IGNORE
            | POSITION_TARGET_TYPEMASK_YAW_IGNORE | POSITION_TARGET_TYPEMASK_YAW_RATE_IGNORE;
    position_command.coordinate_frame = MAV_FRAME_GLOBAL_RELATIVE_ALT_INT;
    position_command.target_system = targetSystem;
    position_command.target_component = targetComponent;

    mavlink_message_t msg = {};
    mavlink_msg_set_position_target_global_int_encode(targetSystem, targetComponent, &msg, &position_command);
    instructions.push_back(
            std::make_shared<Instruction>(msg,
                    TelemetryConditions::getCheckGlobalPosition(latitude, longitude, altitude, tolerance, coordinateSystem, targetSystem), timeout,
                    retries,
                    "Setting GLOBAL_POSITION_INT for GOTO"));

    return instructions;
}

// https://ardupilot.org/dev/docs/plane-commands-in-guided-mode.html
std::vector<std::shared_ptr<Instruction>> guidedGotoPlane(double latitude, double longitude, float altitude, double tolerance,
        inet::IGeographicCoordinateSystem *coordinateSystem, uint8_t targetSystem, uint8_t targetComponent, omnetpp::simtime_t timeout, int retries)
{
    std::vector<std::shared_ptr<Instruction>> instructions;

    mavlink_mission_item_t missionItem = {};
    missionItem.target_system = targetSystem;
    missionItem.target_component = targetComponent;
    missionItem.x = latitude;
    missionItem.y = longitude;
    missionItem.z = altitude;
    missionItem.command = MAV_CMD_NAV_WAYPOINT;
    missionItem.frame = MAV_FRAME_GLOBAL_RELATIVE_ALT;
    missionItem.current = 2; // Current 2 to indicate goto command
    missionItem.autocontinue = 1;

    mavlink_message_t msg = {};
    mavlink_msg_mission_item_encode(targetSystem, targetComponent, &msg, &missionItem);

    instructions.push_back(
            std::make_shared<Instruction>(msg,
                    TelemetryConditions::getCheckGlobalPosition(latitude, longitude, altitude, tolerance, coordinateSystem, targetSystem), timeout,
                    retries, "Sending mission item for GOTO"));

    return instructions;
}

// https://ardupilot.org/dev/docs/copter-commands-in-guided-mode.html
std::vector<std::shared_ptr<Instruction>> guidedGotoRover(double latitude, double longitude, double tolerance,
        inet::IGeographicCoordinateSystem *coordinateSystem, uint8_t targetSystem, uint8_t targetComponent, omnetpp::simtime_t timeout, int retries)
{
    std::vector<std::shared_ptr<Instruction>> instructions;

    mavlink_set_position_target_global_int_t position_command = {};
    position_command.lat_int = latitude * (1e7);
    position_command.lon_int = longitude * (1e7);
    position_command.alt = 0;
    position_command.type_mask = POSITION_TARGET_TYPEMASK_VX_IGNORE | POSITION_TARGET_TYPEMASK_VY_IGNORE | POSITION_TARGET_TYPEMASK_VZ_IGNORE
            | POSITION_TARGET_TYPEMASK_AX_IGNORE | POSITION_TARGET_TYPEMASK_AY_IGNORE | POSITION_TARGET_TYPEMASK_AZ_IGNORE
            | POSITION_TARGET_TYPEMASK_YAW_IGNORE | POSITION_TARGET_TYPEMASK_YAW_RATE_IGNORE;
    position_command.coordinate_frame = MAV_FRAME_GLOBAL_RELATIVE_ALT_INT;
    position_command.target_system = targetSystem;
    position_command.target_component = targetComponent;

    mavlink_message_t msg = {};
    mavlink_msg_set_position_target_global_int_encode(targetSystem, targetComponent, &msg, &position_command);
    instructions.push_back(
            std::make_shared<Instruction>(msg,
                    TelemetryConditions::getCheckGlobalPosition(latitude, longitude, 0, tolerance, coordinateSystem, targetSystem), timeout,
                    retries, "Setting GLOBAL_POSITION_INT for GOTO"));

    return instructions;
}


std::vector<std::shared_ptr<Instruction>> guidedGoto(VehicleType type, double latitude, double longitude, float altitude, double tolerance,
        inet::IGeographicCoordinateSystem *coordinateSystem, uint8_t targetSystem, uint8_t targetComponent, omnetpp::simtime_t timeout, int retries)
{
    switch (type) {
        case COPTER:
            return guidedGotoCopter(latitude, longitude, altitude, tolerance, coordinateSystem, targetSystem, targetComponent, timeout, retries);
        case PLANE:
            return guidedGotoPlane(latitude, longitude, altitude, tolerance, coordinateSystem, targetSystem, targetComponent, timeout, retries);
        case ROVER:
            return guidedGotoRover(latitude, longitude, tolerance, coordinateSystem, targetSystem, targetComponent, timeout, retries);
    }
    return {};
}

}
}
