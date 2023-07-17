#include "mavlink/ardupilotmega/mavlink.h"
#include "inet/common/geometry/common/GeographicCoordinateSystem.h"
#include <functional>
#include "VehicleModes.h"
#include "VehicleTypes.h"

namespace projeto {
// 
// This module contains utility functions used as or to generate Condition functions.
// Conditions are functions used by [MAVLinkMobilityBase](/MAVSIMNET/Modules/MAVLinkMobilityBase/) to check 
// if a MAVLink message is concluded and a new one should be sent to the simulator. They work by receiving incoming telemetry
// on their mavlink_message_t parameter and returning a boolean to flag if that telemetry completes the condition.
namespace TelemetryConditions {
    // An empty condition
    bool checkEmpty(mavlink_message_t);

    // Get a condition that waits for a specific command acknowledge message.
    std::function<bool(mavlink_message_t)> getCheckCmdAck(uint8_t systemId, uint8_t componentId, unsigned short command, uint8_t senderSystemId);

    // Get a condition that checks if the vehicles pre-arm is good
    std::function<bool(mavlink_message_t)> getCheckPreArm(uint8_t senderSystemId);

    // Get a condition that checks if the vehicle is armed
    std::function<bool(mavlink_message_t)> getCheckArm(uint8_t senderSystemId);

    // Get a condition that checks if the vehicle has reached a specific altitude within a tolerance
    std::function<bool(mavlink_message_t)> getCheckAltitude(int32_t altitude, int32_t tolerance, uint8_t senderSystemId);

    // Get a condition that waits for a mission request message with a specific sequence number
    std::function<bool(mavlink_message_t)> getCheckMissionRequest(uint8_t systemid, uint8_t componentId, uint16_t sequenceNumber, uint8_t senderSystemId);

    // Get a condition that waits for a mission ack message
    std::function<bool(mavlink_message_t)> getCheckMissionAck(uint8_t systemId, uint8_t componentId, uint8_t senderSystemId);

    // Get a condition that waits for a mission item reached message
    std::function<bool(mavlink_message_t)> getCheckMissionItemReached(uint16_t seq, uint8_t senderSystemId);

    // Get a condition that waits for a target global message with a specific location
    std::function<bool(mavlink_message_t)> getCheckTargetGlobal(float lat, float lon, float alt, uint8_t senderSystemId);

    // Get a condition that checks if the vehicle has reached a specific condition within tolerance
    std::function<bool(mavlink_message_t)> getCheckGlobalPosition(float lat, float lon, float alt, float tolerance, inet::IGeographicCoordinateSystem *coordinateSystem, uint8_t senderSystemId);

    // Get a condition that checks if a specific parameter has a desired value
    std::function<bool(mavlink_message_t)> getCheckParamValue(std::string param_id, float param_value, uint8_t senderSystemId);

    // Get a condition that checks if the vehicle is on a specific mode
    std::function<bool(mavlink_message_t)> getCheckMode(Mode mode, VehicleType type, uint8_t senderSystemId);
}
}
