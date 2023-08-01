// MESSAGE GROUP_START support class

#pragma once

namespace mavlink {
namespace common {
namespace msg {

/**
 * @brief GROUP_START message
 *
 * Emitted during mission execution when control reaches MAV_CMD_GROUP_START.
 */
struct GROUP_START : mavlink::Message {
    static constexpr msgid_t MSG_ID = 414;
    static constexpr size_t LENGTH = 16;
    static constexpr size_t MIN_LENGTH = 16;
    static constexpr uint8_t CRC_EXTRA = 109;
    static constexpr auto NAME = "GROUP_START";


    uint32_t group_id; /*<  Mission-unique group id (from MAV_CMD_GROUP_START). */
    uint32_t mission_checksum; /*<  CRC32 checksum of current plan for MAV_MISSION_TYPE_ALL. As defined in MISSION_CHECKSUM message. */
    uint64_t time_usec; /*< [us] Timestamp (UNIX Epoch time or time since system boot).
        The receiving end can infer timestamp format (since 1.1.1970 or since system boot) by checking for the magnitude of the number. */


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
        ss << "  group_id: " << group_id << std::endl;
        ss << "  mission_checksum: " << mission_checksum << std::endl;
        ss << "  time_usec: " << time_usec << std::endl;

        return ss.str();
    }

    inline void serialize(mavlink::MsgMap &map) const override
    {
        map.reset(MSG_ID, LENGTH);

        map << time_usec;                     // offset: 0
        map << group_id;                      // offset: 8
        map << mission_checksum;              // offset: 12
    }

    inline void deserialize(mavlink::MsgMap &map) override
    {
        map >> time_usec;                     // offset: 0
        map >> group_id;                      // offset: 8
        map >> mission_checksum;              // offset: 12
    }
};

} // namespace msg
} // namespace common
} // namespace mavlink
