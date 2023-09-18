from simulator.protocols.IProtocol import IProtocol
from simulator.messages.CommunicationCommand import SendMessageCommand
from simulator.messages.Telemetry import Telemetry
from simulator.protocols.simple.SimpleMessage import SenderType, SimpleMessage


class SimpleProtocolGround(IProtocol):
    packets: int

    def initialize(self, stage: int):
        self.packets = 0
        self.provider.tracked_variables["packets"] = self.packets

    def handle_timer(self, timer: dict):
        pass

    def handle_packet(self, message: str):
        message: SimpleMessage = SimpleMessage.from_json(message)
        print(f"SimpleProtocolGround received packet: {self.packets}, {message.sender}")

        if message.sender == SenderType.DRONE:
            self.packets += message.content
            self.provider.tracked_variables["packets"] = self.packets
            
            response = SimpleMessage(
                sender=SenderType.GROUND_STATION, content=self.packets
            )
            self.provider.send_communication_command(
                SendMessageCommand(response.to_json())
            )

    def handle_telemetry(self, telemetry: Telemetry):
        pass

    def finish(self):
        pass
