// MESSAGE MANUAL_CONTROL support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief MANUAL_CONTROL message
 *
 * This message provides an API for manually controlling the vehicle using standard joystick axes nomenclature, along with a joystick-like input device. Unused axes can be disabled and buttons states are transmitted as individual on/off bits of a bitmask
 */
struct MANUAL_CONTROL : mavlink::Message {
    static constexpr msgid_t MSG_ID = 69;
    static constexpr size_t LENGTH = 18;
    static constexpr size_t MIN_LENGTH = 11;
    static constexpr uint8_t CRC_EXTRA = 243;
    static constexpr auto NAME = "MANUAL_CONTROL";


    uint8_t target; /*<  The system to be controlled. */
    int16_t x; /*<  X-axis, normalized to the range [-1000,1000]. A value of INT16_MAX indicates that this axis is invalid. Generally corresponds to forward(1000)-backward(-1000) movement on a joystick and the pitch of a vehicle. */
    int16_t y; /*<  Y-axis, normalized to the range [-1000,1000]. A value of INT16_MAX indicates that this axis is invalid. Generally corresponds to left(-1000)-right(1000) movement on a joystick and the roll of a vehicle. */
    int16_t z; /*<  Z-axis, normalized to the range [-1000,1000]. A value of INT16_MAX indicates that this axis is invalid. Generally corresponds to a separate slider movement with maximum being 1000 and minimum being -1000 on a joystick and the thrust of a vehicle. Positive values are positive thrust, negative values are negative thrust. */
    int16_t r; /*<  R-axis, normalized to the range [-1000,1000]. A value of INT16_MAX indicates that this axis is invalid. Generally corresponds to a twisting of the joystick, with counter-clockwise being 1000 and clockwise being -1000, and the yaw of a vehicle. */
    uint16_t buttons; /*<  A bitfield corresponding to the joystick buttons' 0-15 current state, 1 for pressed, 0 for released. The lowest bit corresponds to Button 1. */
    uint16_t buttons2; /*<  A bitfield corresponding to the joystick buttons' 16-31 current state, 1 for pressed, 0 for released. The lowest bit corresponds to Button 16. */
    uint8_t enabled_extensions; /*<  Set bits to 1 to indicate which of the following extension fields contain valid data: bit 0: pitch, bit 1: roll. */
    int16_t s; /*<  Pitch-only-axis, normalized to the range [-1000,1000]. Generally corresponds to pitch on vehicles with additional degrees of freedom. Valid if bit 0 of enabled_extensions field is set. Set to 0 if invalid. */
    int16_t t; /*<  Roll-only-axis, normalized to the range [-1000,1000]. Generally corresponds to roll on vehicles with additional degrees of freedom. Valid if bit 1 of enabled_extensions field is set. Set to 0 if invalid. */


    inline std::string get_name(void) const override
    {
            return NAME;
    }

    inline Info get_message_info(void) const override
    {
            return { MSG_ID, LENGTH, MIN_LENGTH, CRC_EXTRA };
    }

    inline std::string to_yaml(void) const override
    {
        std::stringstream ss;

        ss << NAME << ":" << std::endl;
        ss << "  target: " << +target << std::endl;
        ss << "  x: " << x << std::endl;
        ss << "  y: " << y << std::endl;
        ss << "  z: " << z << std::endl;
        ss << "  r: " << r << std::endl;
        ss << "  buttons: " << buttons << std::endl;
        ss << "  buttons2: " << buttons2 << std::endl;
        ss << "  enabled_extensions: " << +enabled_extensions << std::endl;
        ss << "  s: " << s << std::endl;
        ss << "  t: " << t << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << x;                             // offset: 0
        map << y;                             // offset: 2
        map << z;                             // offset: 4
        map << r;                             // offset: 6
        map << buttons;                       // offset: 8
        map << target;                        // offset: 10
        map << buttons2;                      // offset: 11
        map << enabled_extensions;            // offset: 13
        map << s;                             // offset: 14
        map << t;                             // offset: 16
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> x;                             // offset: 0
        map >> y;                             // offset: 2
        map >> z;                             // offset: 4
        map >> r;                             // offset: 6
        map >> buttons;                       // offset: 8
        map >> target;                        // offset: 10
        map >> buttons2;                      // offset: 11
        map >> enabled_extensions;            // offset: 13
        map >> s;                             // offset: 14
        map >> t;                             // offset: 16
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
