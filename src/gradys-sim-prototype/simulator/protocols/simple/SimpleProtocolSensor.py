from simulator.protocols.IProtocol import IProtocol
from simulator.provider.IProvider import IProvider
from simulator.messages.CommunicationCommand import SendMessageCommand
from simulator.messages.Telemetry import Telemetry
from simulator.protocols.simple.SimpleMessage import SimpleMessage, SenderType


class SimpleProtocolSensor(IProtocol):
    packets: int
    provider: IProvider

    def initialize(self, stage: int):
        self.packets = 0
        self.provider.schedule_timer({}, self.provider.current_time() + 1)

    def handle_timer(self, timer: dict):
        self.packets += 1
        self.provider.schedule_timer({}, self.provider.current_time() + 1)

    def handle_packet(self, message: SimpleMessage):
        if message.sender == SenderType.DRONE:
            response: SimpleMessage(sender=SenderType.SENSOR, content=self.packets)
            self.provider.send_communication_command(SendMessageCommand(response))
            self.packets = 0

    def handle_telemetry(self, telemetry: Telemetry):
        pass

    def finish(self):
        pass
