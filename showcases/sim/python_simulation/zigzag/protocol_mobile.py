import logging
import os
import random
from pathlib import Path
from gradysim.protocol.plugin.mission_mobility import (
    LoopMission,
    MissionMobilityPlugin,
    MissionMobilityConfiguration,
)
from gradysim.protocol.messages.communication import BroadcastMessageCommand, SendMessageCommand
from gradysim.protocol.messages.telemetry import Telemetry
from gradysim.protocol.interface import IProtocol
from message import ZigZagMessage, ZigZagMessageType, ZigZagNodeType
from utils import CommunicationStatus

delays = 0

class ZigZagProtocolMobile(IProtocol):
    def __init__(self):
        self.timeout_end: int = 0
        self.timeout_set: bool = False
        self.timeout_duration: int = 5

        self.communication_status: CommunicationStatus = CommunicationStatus.FREE

        self.tentative_target: int = -1

        self.current_data_load: int = 0
        self.stable_data_load: int = self.current_data_load

        self.current_telemetry: Telemetry
        self.last_stable_telemetry: Telemetry
       
        self._logger = logging.getLogger()
        self.old_mission_is_reversed = False

        self.mission_started = False


    def initialize(self):
        self._logger.debug("initializing mobile protocol")

        # self.provider.tracked_variables["timeout_set"] = self.timeout_set
        # self.provider.tracked_variables["timeout_end"] = self.timeout_end
        self.provider.tracked_variables["current_data_load"] = self.current_data_load
        # self.provider.tracked_variables["stable_data_load"] = self.current_data_load
        # self.provider.tracked_variables["communication_status"] = self.communication_status.name

        self.mission: MissionMobilityPlugin = MissionMobilityPlugin(
            self, MissionMobilityConfiguration(loop_mission=LoopMission.REVERSE, speed=10)
        )

        global delays
        self.provider.schedule_timer("START_MISSION",
                                     self.provider.current_time() + delays)
        
        delays = delays + 60
        print(f"ID: {self.provider.get_id()}")


        self.provider.schedule_timer("", self.provider.current_time() + 5)

    def handle_timer(self, timer: str):
        if timer == "START_MISSION":
            path = Path(__file__).parent / "mission.txt"
            self.mission.start_mission_with_waypoint_file(mission_file_path=path)
            self.mission_started = True
            return

        self._send_heartbeat()
        self.provider.schedule_timer("", self.provider.current_time() + 5)

    def handle_packet(self, message: str):
        if not self.mission_started:
            return

        self._logger.debug("Handling packet in mobile protocol")
        message: ZigZagMessage = ZigZagMessage.from_json(message)

        if message.message_type == ZigZagMessageType.HEARTBEAT:
            if not self._is_timedout():
                self.tentative_target = message.source_id
                self.old_mission_is_reversed = self.mission.is_reversed
                self._send_message()
        elif message.message_type == ZigZagMessageType.PAIR_REQUEST:
                if self._is_timedout() or message.destination_id != self.provider.get_id():
                    return
                else: 
                    if self.communication_status != CommunicationStatus.PAIRED:
                        self.tentative_target = message.source_id
                        self.communication_status = CommunicationStatus.PAIRED
                        self.old_mission_is_reversed = self.mission.is_reversed
                        self._send_message()

        elif message.message_type == ZigZagMessageType.PAIR_CONFIRM:
                if self._is_timedout() or message.destination_id != self.provider.get_id():
                    return 
                else:
                    if message.source_id == self.tentative_target:
                        if self.communication_status != CommunicationStatus.PAIRED_FINISHED:
                            self._initiate_timeout() 

                            self.old_mission_is_reversed = self.mission.is_reversed

                            if message.source_node_type == ZigZagNodeType.GROUND:
                                self.mission.set_reversed(False)
                            else:
                                if not self.mission.is_reversed:
                                    # Drone flying into mission direction
                                    if message.reversed_flag:
                                        # Drone flying in reverse order of mission direction
                                        reversed = not self.mission.is_reversed
                                        self.mission.set_reversed(reversed)
                                    else:
                                        # Drone flying in mission direction
                                        if self.provider.get_id() > message.source_id:
                                            reversed = not self.mission.is_reversed
                                            self.mission.set_reversed(reversed)

                                else:
                                    # Drone flying in reverse order of mission direction
                                    if message.reversed_flag:
                                        # Drone flying in reverse order of mission direction
                                        if self.provider.get_id() > message.source_id:
                                            reversed = not self.mission.is_reversed
                                            self.mission.set_reversed(reversed)

                                    else:
                                        # Drone flying in reverse order of mission direction
                                        reversed = not self.mission.is_reversed
                                        self.mission.set_reversed(reversed)

                            if self.mission.is_reversed:
                                self.current_data_load += message.data_length
                            else:
                                self.current_data_load = 0
                            self.provider.tracked_variables["current_data_load"] = self.current_data_load

                            self.communication_status = CommunicationStatus.PAIRED_FINISHED
                            self._send_message()

        elif message.message_type ==  ZigZagMessageType.BEARER:
            self._logger.debug("Exchanging data in mobile protocol")
            # Only used to exchange information between drone and sensor
            self.current_data_load = self.current_data_load + message.data_length
            self.stable_data_load = self.current_data_load
            self.provider.tracked_variables["current_data_load"] = self.current_data_load
            self.provider.tracked_variables["stable_data_load"] = self.stable_data_load

    def _send_message(self):
        message = ZigZagMessage(
            source_id=self.provider.get_id(),
            reversed_flag=self.old_mission_is_reversed,
            source_node_type=ZigZagNodeType.MOBILE
        )

        if self.provider.get_id() == self.tentative_target:
            return
        
        if self.communication_status == CommunicationStatus.FREE:
            message.message_type = ZigZagMessageType.PAIR_REQUEST
            message.destination_id = self.tentative_target

        elif self.communication_status == CommunicationStatus.PAIRED:
            message.message_type = ZigZagMessageType.PAIR_CONFIRM
            message.destination_id = self.tentative_target
            message.data_length = self.stable_data_load

        elif self.communication_status == CommunicationStatus.PAIRED_FINISHED:
            message.message_type = ZigZagMessageType.PAIR_FINISH
            message.destination_id = self.tentative_target
            message.data_length = self.stable_data_load

        self.provider.tracked_variables["communication_status"] = self.communication_status.name

        if self.tentative_target < 0:
            command = BroadcastMessageCommand(
                message=message.to_json()
            )

        else:
            command = SendMessageCommand(
                message=message.to_json(), destination=self.tentative_target
            )

        self.provider.send_communication_command(command)

    def handle_telemetry(self, telemetry: Telemetry):
        self.current_telemetry = telemetry

        if self._is_timedout():
            self.last_stable_telemetry = telemetry

    def finish(self):
        pass

    def _send_heartbeat(self):
        message = ZigZagMessage(
            source_id=self.provider.get_id(),
            reversed_flag=self.mission.is_reversed,
            message_type=ZigZagMessageType.HEARTBEAT,
            source_node_type=ZigZagNodeType.MOBILE
        )

        self.provider.tracked_variables["communication_status"] = self.communication_status.name

        command = BroadcastMessageCommand(
            message=message.to_json()
        )
        self.provider.send_communication_command(command)

    def _initiate_timeout(self):
        if self.timeout_duration > 0:
            self.timeout_end = self.provider.current_time() + self.timeout_duration
            self.timeout_set = True

    def _is_timedout(self):
        def __is_timedout():
            if self.timeout_set:
                if self.provider.current_time() < self.timeout_end:
                    return True
                else:
                    self.timeout_set = False
                    return False
            else:
                return False
    
        self.provider.tracked_variables["timeout_set"] = self.timeout_set
        self.provider.tracked_variables["timeout_end"] = self.timeout_end

        old_timeout_set = self.timeout_set
        is_timedout = __is_timedout()
        if not is_timedout and old_timeout_set:
            self._reset_parameters()
        return is_timedout

           
    def _reset_parameters(self):
        self.timeout_set = False
        self.tentative_target = -1
        self.communication_status = CommunicationStatus.FREE
        self.last_stable_telemetry = self.current_telemetry
        self.stable_data_load = self.current_data_load
        self.provider.tracked_variables["stable_data_load"] = self.stable_data_load
