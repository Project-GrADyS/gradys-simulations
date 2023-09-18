import random
from simulator.protocols.IProtocol import IProtocol
from simulator.messages.CommunicationCommand import SendMessageCommand
from simulator.messages.MobilityCommand import (
    SetModeCommand,
    MobilityMode,
    ReverseCommand,
)
from simulator.messages.Telemetry import Telemetry
from simulator.protocols.simple.SimpleMessage import SenderType, SimpleMessage


class SimpleProtocolMobile(IProtocol):
    packets: int
    last_telemetry_message: Telemetry

    def initialize(self, stage: int):
        self.packets = 0
        self.last_telemetry_message = Telemetry()
        self.provider.send_mobility_command(SetModeCommand(MobilityMode.AUTO))

        # Scheduling self message with a random delay to prevent collision when sending pings
        self.provider.tracked_variables["packets"] = self.packets
        self.provider.schedule_timer({}, self.provider.current_time() + random.random())

    def handle_timer(self, timer: dict):
        ping = SimpleMessage(sender=SenderType.DRONE, content=self.packets)
        self.provider.send_communication_command(SendMessageCommand(ping.to_json()))
        self.provider.schedule_timer({}, self.provider.current_time() + 2)

    def handle_packet(self, message: str):
        message: SimpleMessage = SimpleMessage.from_json(message)
        print(
            f"SimpleProtocolMobile received packet: {self.packets}, {message.sender}, {self.last_telemetry_message.is_reversed}"
        )

        if message.sender == SenderType.GROUND_STATION:
            self.packets = 0
            self.provider.tracked_variables["packets"] = self.packets

            if self.last_telemetry_message.is_reversed:
                self.provider.send_mobility_command(ReverseCommand())

        elif message.sender == SenderType.SENSOR:
            self.packets += message.content
            self.provider.tracked_variables["packets"] = self.packets

    def handle_telemetry(self, telemetry: Telemetry):
        self.last_telemetry_message = telemetry

    def finish(self):
        pass
