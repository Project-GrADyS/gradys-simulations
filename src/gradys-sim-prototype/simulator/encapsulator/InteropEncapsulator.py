from enum import Enum
from typing import Tuple, Union, List, Type

from simulator.encapsulator.IEncapsulator import IEncapsulator
from simulator.messages.CommunicationCommand import CommunicationCommand
from simulator.messages.MobilityCommand import MobilityCommand
from simulator.protocols.IProtocol import IProtocol
from simulator.provider.IProvider import IProvider


class _ConsequenceType(Enum):
    COMMUNICATION = 1
    MOBILITY = 2
    TIMER = 3


_TimerParams = Tuple[dict, float]

_Consequence = Tuple[_ConsequenceType, Union[CommunicationCommand, MobilityCommand, _TimerParams]]


class _InteropProvider(IProvider):
    consequences: List[_Consequence]
    timestamp: int

    def __init__(self):
        self.consequences = []
        self.timestamp = 0

    def send_communication_command(self, command: CommunicationCommand):
        self.consequences.append((_ConsequenceType.COMMUNICATION, command))

    def send_mobility_command(self, command: MobilityCommand):
        self.consequences.append((_ConsequenceType.MOBILITY, command))

    def schedule_timer(self, timer: dict, timestamp: float):
        self.consequences.append((_ConsequenceType.TIMER, (timer, timestamp)))

    def current_time(self) -> int:
        return self.timestamp


class InteropEncapsulator(IEncapsulator):
    provider: _InteropProvider

    @classmethod
    def encapsulate(cls, protocol: Type[IProtocol]):
        encapsulator = cls()

        encapsulator.provider = _InteropProvider()
        encapsulator.protocol = protocol.instantiate(encapsulator.provider)
        return encapsulator

    def _collect_consequences(self) -> List[_Consequence]:
        consequences = self.provider.consequences
        self.provider.consequences = []
        return consequences

    def set_timestamp(self, timestamp: int):
        self.provider.timestamp = timestamp

    def initialize(self, stage: int) -> List[_Consequence]:
        self.protocol.initialize(stage)
        return self._collect_consequences()

    def handle_timer(self, timer: dict) -> List[_Consequence]:
        self.protocol.handle_timer(timer)
        return self._collect_consequences()

    def handle_packet(self, message: dict) -> List[_Consequence]:
        self.protocol.handle_packet(message)
        return self._collect_consequences()

    def finish(self) -> List[_Consequence]:
        self.protocol.finish()
        return self._collect_consequences()
