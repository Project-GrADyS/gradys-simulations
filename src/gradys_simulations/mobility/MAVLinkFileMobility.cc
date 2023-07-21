//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include <fstream>
#include "MAVLinkFileMobility.h"
#include "gradys_simulations/utils/TelemetryConditions.h"
#include "gradys_simulations/utils/VehicleRoutines.h"


using namespace omnetpp;

namespace gradys_simulations {

Define_Module(MAVLinkFileMobility);

void MAVLinkFileMobility::initialize(int stage)
{
    MAVLinkMobilityBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        waypointFile = par("waypointFile").stdstringValue();
    }
    if (stage == 1) {
        readMissionFromFile();
        startMission();
    }
}

mavlink_command_long_t MAVLinkFileMobility::vectorToCommand(std::vector<std::string> &vector) {
    mavlink_command_long_t command = {};
    command.param1 = std::stod(vector[4]);
    command.param2 = std::stod(vector[5]);
    command.param3 = std::stod(vector[6]);
    command.param4 = std::stod(vector[7]);
    command.param5 = std::stod(vector[8]);
    command.param6 = std::stod(vector[9]);
    command.param7 = std::stod(vector[10]);
    command.command = std::stoi(vector[3]);
    command.target_system = targetSystem;
    command.target_component = targetComponent;
    return command;
}

void MAVLinkFileMobility::readMissionFromFile() {
    EV_INFO << "READING WAYPOINT FILE" << std::endl;
    char line[512];
    std::ifstream inputFile(waypointFile);

    if(!inputFile.getline(line, 256) || std::strcmp(line, "QGC WPL 110") != 0) {
        EV_ERROR << "Incorrect waypoint file format: " << waypointFile << " does not have QGC WPL 110 header." << std::endl;
        return;
    }

    std::vector<mavlink_command_long_t> commands;
    std::vector<std::vector<std::string>> lines;
    while (inputFile.getline(line, 512)) {
        cStringTokenizer tokenizer(line, "\t");
        std::vector<std::string> lineVector = tokenizer.asVector();

        if (lineVector.size() != 12) {
            EV_ERROR << "Waypoint file format error." << endl;
            return;
        }
        commands.push_back(vectorToCommand(lineVector));
        std::vector<std::string> copyVector(lineVector);
        lines.push_back(copyVector);
    }

    mavlink_mission_count_t missionRequest = {};
    missionRequest.count = commands.size();
    missionRequest.target_system = targetSystem;
    missionRequest.target_component = targetComponent;
    missionRequest.mission_type = MAV_MISSION_TYPE_MISSION;

    mavlink_message_t missionRequestMessage = {};
    mavlink_msg_mission_count_encode(systemId, componentId, &missionRequestMessage, &missionRequest);
    queueMessage(missionRequestMessage, TelemetryConditions::getCheckMissionRequest(systemId, componentId, 0, targetSystem), 15, 3);

    mavlink_mission_item_int_t missionItem = {};
    missionItem.target_system = targetSystem;
    missionItem.target_component = targetComponent;
    missionItem.mission_type = MAV_MISSION_TYPE_MISSION;

    mavlink_message_t commandMessage = {};
    uint16_t index = 0;
    for(mavlink_command_long_t &command : commands) {
        missionItem.param1 = command.param1;
        missionItem.param2 = command.param2;
        missionItem.param3 = command.param3;
        missionItem.param4 = command.param4;
        missionItem.x = static_cast<int32_t>(command.param5 * 1e7);
        missionItem.y = static_cast<int32_t>(command.param6 * 1e7);
        missionItem.z = command.param7;
        missionItem.seq = std::stoi(lines[index][0]);
        missionItem.command = command.command;
        missionItem.frame = std::stoi(lines[index][2]);
        missionItem.current = std::stoi(lines[index][1]);
        missionItem.autocontinue = std::stoi(lines[index][11]);

        mavlink_msg_mission_item_int_encode(systemId, componentId, &commandMessage, &missionItem);
        if(index == (commands.size() - 1)) {
            queueMessage(commandMessage, TelemetryConditions::getCheckMissionAck(systemId, componentId, targetSystem), 15, 3);
        } else {
            queueMessage(commandMessage, TelemetryConditions::getCheckMissionRequest(systemId, componentId, index + 1, targetSystem), 15, 3);
        }

        index++;
    }
}


void MAVLinkFileMobility::startMission() {
    mavlink_command_long_t cmd = {};
    mavlink_message_t message = {};

    // Sending MODE GUIDED command
    queueInstructions(VehicleRoutines::setMode(systemId, componentId, vehicleType, GUIDED, targetSystem, targetComponent));

    // Sending ARM THROTTLE command
    cmd = {};
    cmd.command = MAV_CMD_COMPONENT_ARM_DISARM;
    cmd.confirmation = 0;
    cmd.param1 = 1;
    cmd.target_component = targetComponent;
    cmd.target_system = targetSystem;
    mavlink_msg_command_long_encode(targetSystem, targetComponent, &message, &cmd);
    queueMessage(message, TelemetryConditions::getCheckArm(targetSystem), 15, 3);

    // Sending MODE AUTO command
    queueInstructions(VehicleRoutines::setMode(systemId, componentId, vehicleType, AUTO, targetSystem, targetComponent));

    // Starting mission
    cmd = {};
    cmd.command = MAV_CMD_MISSION_START;
    cmd.confirmation = 0;
    cmd.target_component = targetComponent;
    cmd.target_system = targetSystem;
    mavlink_msg_command_long_encode(targetSystem, targetComponent, &message, &cmd);
    queueMessage(message, TelemetryConditions::getCheckCmdAck(targetSystem, targetComponent, MAV_CMD_MISSION_START, targetSystem), 15, 3);
}

}
