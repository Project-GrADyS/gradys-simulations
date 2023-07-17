#include <iostream>

#include "TelemetryConditions.h"

/**
 * @brief This module contains message checkers that should be used as conditions for requirements.
 * 
 */
namespace projeto {
namespace TelemetryConditions {

bool verifySender(mavlink_message_t message, uint8_t senderSystemId) {
    return (message.sysid == senderSystemId);
}


/**
 * @brief An empty checker, always immediatly resolves
 * 
 * @return true 
 */
bool checkEmpty(mavlink_message_t) {
    return true;
}


/**
 * @brief Gets the check_cmd_ack message checker for a specific command
 * 
 * @param command The command you want to get the checker for
 * @return Condition Returns true when command gets acknowledged
 */
std::function<bool(mavlink_message_t)> getCheckCmdAck(uint8_t systemId, uint8_t componentId, unsigned short command, uint8_t senderSystemId) {
    return [=](mavlink_message_t message) {
        if(message.msgid == MAVLINK_MSG_ID_COMMAND_ACK && verifySender(message, senderSystemId)) {
            mavlink_command_ack_t ack = {};
            mavlink_msg_command_ack_decode(&message, &ack);
            return (ack.command == command && ack.result == MAV_RESULT_ACCEPTED && (ack.target_system == systemId || ack.target_system == 0));
        }
        return false;
    };
}

/**
 * @brief Checks if the vehicle is ready to arm
 * 
 * @param message Telemetry recieved
 * @return true If the vehicle is ready to arm
 * @return false If the vehicle is not yet ready to arm 
 */
std::function<bool(mavlink_message_t)> getCheckPreArm(uint8_t senderSystemId) {
    static const uint16_t required_value = (EKF_ATTITUDE |
                ESTIMATOR_VELOCITY_HORIZ |
                ESTIMATOR_VELOCITY_VERT |
                ESTIMATOR_POS_HORIZ_REL |
                ESTIMATOR_PRED_POS_HORIZ_REL);
                    
    static const uint16_t error_bits = (ESTIMATOR_CONST_POS_MODE |
                ESTIMATOR_ACCEL_ERROR);
    return [=](mavlink_message_t message) {
        if(message.msgid == MAVLINK_MSG_ID_EKF_STATUS_REPORT && verifySender(message, senderSystemId)) {
            mavlink_ekf_status_report_t report = {};
            mavlink_msg_ekf_status_report_decode(&message, &report);
            // Flags don't include error bits and include all required values
            return !(report.flags & error_bits) && ((report.flags & required_value) == required_value);
        }
        return false;
    };
}

/**
 * @brief Checks if the vehicle is armed
 * 
 * @param message Telemetry received
 * @return true If the vehicle is armed
 * @return false If the vehicle is not armed
 */
std::function<bool(mavlink_message_t)> getCheckArm(uint8_t senderSystemId) {
    return [=](mavlink_message_t message) {
        if(message.msgid == MAVLINK_MSG_ID_HEARTBEAT && verifySender(message, senderSystemId)) {
            mavlink_heartbeat_t heartbeat = {};
            mavlink_msg_heartbeat_decode(&message, &heartbeat);
            return static_cast<bool>(heartbeat.base_mode & MAV_MODE_FLAG_SAFETY_ARMED);
        }
        return false;
    };
}

/**
 * @brief Gets a message checker that will verify if a vehicle is within tolerance of an altutude
 * 
 * @param altitude Target altitude
 * @param tolerance Margin of error for the altitude
 * @return Condition Message checker for that specific altitude and tolerance
 */
std::function<bool(mavlink_message_t)> getCheckAltitude(int32_t altitude, int32_t tolerance, uint8_t senderSystemId) {
    return [=](mavlink_message_t msg) {
        if(msg.msgid == MAVLINK_MSG_ID_GLOBAL_POSITION_INT && verifySender(msg, senderSystemId)) {
            mavlink_global_position_int_t position = {};
            mavlink_msg_global_position_int_decode(&msg, &position);
            return abs(position.relative_alt / 1000 - altitude) < tolerance;
        }
        return false;
    };
}

/**
 * @brief Gets a message checker that will resolve when a mission request arrives
 *
 */
std::function<bool(mavlink_message_t)> getCheckMissionRequest(uint8_t systemId, uint8_t componentId, uint16_t sequenceNumber, uint8_t senderSystemId) {
    return [=](mavlink_message_t msg) {
        if(msg.msgid == MAVLINK_MSG_ID_MISSION_REQUEST_INT && verifySender(msg, senderSystemId)) {
            mavlink_mission_request_int_t request = {};
            mavlink_msg_mission_request_int_decode(&msg, &request);
            return (request.seq == sequenceNumber) && (request.target_system == systemId) && (request.target_component == componentId);
        }
        else if(msg.msgid == MAVLINK_MSG_ID_MISSION_REQUEST && verifySender(msg, senderSystemId)) {
            mavlink_mission_request_t request = {};
            mavlink_msg_mission_request_decode(&msg, &request);
            return (request.seq == sequenceNumber) && (request.target_system == systemId || request.target_system == 0);
        }
        return false;
    };
}

std::function<bool(mavlink_message_t)> getCheckMissionAck(uint8_t systemId, uint8_t componentId, uint8_t senderSystemId) {
    return [=](mavlink_message_t msg) {
        if(msg.msgid == MAVLINK_MSG_ID_MISSION_ACK && verifySender(msg, senderSystemId)) {
                mavlink_mission_ack_t ack = {};
                mavlink_msg_mission_ack_decode(&msg, &ack);
                return (ack.type == MAV_MISSION_ACCEPTED) && (ack.target_system == systemId || ack.target_system == 0) ;
        }
        return false;
    };
}

std::function<bool(mavlink_message_t)> getCheckMissionItemReached(uint16_t seq, uint8_t senderSystemId) {
    return [=](mavlink_message_t msg) {
        if(msg.msgid == MAVLINK_MSG_ID_MISSION_ITEM_REACHED && verifySender(msg, senderSystemId)) {
            mavlink_mission_item_reached_t reached = {};
            mavlink_msg_mission_item_reached_decode(&msg, &reached);
            return reached.seq == seq;
        }
        return false;
    };
}

std::function<bool(mavlink_message_t)> getCheckTargetGlobal(float lat, float lon, float alt, uint8_t senderSystemId) {
    return [=](mavlink_message_t msg) {
        if(msg.msgid == MAVLINK_MSG_ID_POSITION_TARGET_GLOBAL_INT && verifySender(msg, senderSystemId)) {
            mavlink_position_target_global_int_t position = {};
            mavlink_msg_position_target_global_int_decode(&msg, &position);

            return (position.lat_int == lat) && (position.lon_int == lon) && (position.alt = alt);
        }
        return false;
    };
}

std::function<bool(mavlink_message_t)> getCheckGlobalPosition(float lat, float lon, float alt, float tolerance, inet::IGeographicCoordinateSystem *coordinateSystem, uint8_t senderSystemId) {
    inet::Coord targetCoords = coordinateSystem->computeSceneCoordinate(inet::GeoCoord(inet::deg(lat), inet::deg(lon), inet::m(alt)));
    return [=](mavlink_message_t msg) {
        if(msg.msgid == MAVLINK_MSG_ID_GLOBAL_POSITION_INT && verifySender(msg, senderSystemId)) {
            mavlink_global_position_int_t position = {};
            mavlink_msg_global_position_int_decode(&msg,&position);

            inet::GeoCoord currentCoord(inet::deg(position.lat/1e7), inet::deg(position.lon/1e7), inet::m(position.relative_alt / 1e3));
            inet::Coord currentSceneCoords = coordinateSystem->computeSceneCoordinate(currentCoord);
            // std::cout << "Vehicle " << +senderSystemId << " - Current: (" << currentSceneCoords.z << ", " << currentSceneCoords.y << ", " << currentSceneCoords.z << ") - Target: (" << targetCoords.x << ", " << targetCoords.y << ", " << targetCoords.z << ") - Distance: " << coordinateSystem->computeSceneCoordinate(currentCoord).distance(targetCoords) << " Tolerance - " << tolerance << std::endl;
            return currentSceneCoords.distance(targetCoords) <= tolerance;
        }
        return false;
    };
}

std::function<bool(mavlink_message_t)> getCheckParamValue(std::string param_id, float param_value, uint8_t senderSystemId) {
    return [=](mavlink_message_t msg) {
        if(msg.msgid == MAVLINK_MSG_ID_PARAM_VALUE && verifySender(msg, senderSystemId)) {
            mavlink_param_value_t param = {};
            mavlink_msg_param_value_decode(&msg, &param);
            return (strcmp(param.param_id, param_id.c_str()) == 0) && (param.param_value == param_value);
        }
        return false;
    };
}

std::function<bool(mavlink_message_t)> getCheckMode(Mode mode, VehicleType type, uint8_t senderSystemId) {
    return [=](mavlink_message_t msg) {
        if(msg.msgid == MAVLINK_MSG_ID_HEARTBEAT && verifySender(msg, senderSystemId)) {
            mavlink_heartbeat_t heartbeat = {};
            mavlink_msg_heartbeat_decode(&msg, &heartbeat);
            if (mode == GUIDED) {
                switch(type) {
                    case COPTER:
                        return heartbeat.custom_mode == COPTER_MODE_GUIDED;
                    case PLANE:
                        return heartbeat.custom_mode == PLANE_MODE_GUIDED;
                    case ROVER:
                        return heartbeat.custom_mode == ROVER_MODE_GUIDED;
                }
            } else if (mode == AUTO) {
                switch(type) {
                    case COPTER:
                        return heartbeat.custom_mode == COPTER_MODE_AUTO;
                    case PLANE:
                        return heartbeat.custom_mode == PLANE_MODE_AUTO;
                    case ROVER:
                        return heartbeat.custom_mode == ROVER_MODE_AUTO;
                }
            } else if (mode == TAKEOFF) {
                switch(type) {
                    case PLANE:
                        return heartbeat.custom_mode == PLANE_MODE_TAKEOFF;
                }
            }
        }
        return false;
    };
}

}
}
