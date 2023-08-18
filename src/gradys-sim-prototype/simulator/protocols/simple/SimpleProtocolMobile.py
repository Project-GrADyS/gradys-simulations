import random
from simulator.protocols.IProtocol import IProtocol
from simulator.messages.CommunicationCommand import SendMessageCommand
from simulator.messages.MobilityCommand import SetModeCommand, MobilityMode, ReverseCommand
from simulator.messages.Telemetry import Telemetry
from simulator.protocols.simple.SimpleMessage import SimpleMessage, SenderType


class SimpleProtocolMobile(IProtocol):
    packets: int

    def initialize(self, stage: int):
        self.packets = 0
        self.provider.send_mobility_command(SetModeCommand(MobilityMode.AUTO))

        # Scheduling self message with a random delay to prevent collision when sending pings
        self.provider.schedule_timer({}, self.provider.current_time() + random.random())

    def handle_timer(self, timer: dict):
        ping: SimpleMessage = {
            'sender': SenderType.DRONE,
            'content': self.packets
        }
        self.provider.send_communication_command(SendMessageCommand(ping))
        self.provider.schedule_timer({}, self.provider.current_time() + 1)

    def handle_packet(self, message: SimpleMessage):
        print("Test", message['sender'])
        if message['sender'] == SenderType.GROUND_STATION:
            self.packets = 0
            self.provider.send_mobility_command(ReverseCommand())

        elif message['sender'] == SenderType.SENSOR:
            self.packets += message['content']

    def handle_telemetry(self, telemetry: Telemetry):
        pass

    def finish(self):
        pass
