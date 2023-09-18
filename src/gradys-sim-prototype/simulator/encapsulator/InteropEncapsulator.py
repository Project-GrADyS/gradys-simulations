from enum import Enum
from typing import Tuple, Union, List, Type, Callable, Any

from simulator.encapsulator.IEncapsulator import IEncapsulator
from simulator.messages.CommunicationCommand import CommunicationCommand
from simulator.messages.MobilityCommand import MobilityCommand
from simulator.messages.Telemetry import Telemetry
from simulator.protocols.IProtocol import IProtocol
from simulator.provider.IProvider import IProvider


class _ConsequenceType(Enum):
    COMMUNICATION = 1
    MOBILITY = 2
    TIMER = 3
    TRACK_VARIABLE = 4


_TimerParams = Tuple[dict, float]

_TrackVariableParams = Tuple[str, Any]

_Consequence = Tuple[
    str,
    Union[CommunicationCommand, MobilityCommand, _TimerParams, _TrackVariableParams],
]


class _TrackedVariableContainer(dict):
    def __init__(self, setter_callback: Callable[[str, Any], None]):
        super().__init__()
        self.callback = setter_callback

    def __setitem__(self, key: str, value: Any):
        self.callback(key, value)


class _InteropProvider(IProvider):
    consequences: List[_Consequence]
    timestamp: float

    def __init__(self):
        self.consequences = []
        self.timestamp = 0
        self.tracked_variables = _TrackedVariableContainer(
            lambda key, value: self.consequences.append(
                (_ConsequenceType.TRACK_VARIABLE.name, (key, str(value)))
            )
        )

    def send_communication_command(self, command: CommunicationCommand):
        self.consequences.append((_ConsequenceType.COMMUNICATION.name, command))

    def send_mobility_command(self, command: MobilityCommand):
        self.consequences.append((_ConsequenceType.MOBILITY.name, command))

    def schedule_timer(self, timer: dict, timestamp: float):
        self.consequences.append((_ConsequenceType.TIMER.name, (timer, timestamp)))

    def current_time(self) -> float:
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

    def set_timestamp(self, timestamp: float):
        self.provider.timestamp = timestamp

    def initialize(self, stage: int) -> List[_Consequence]:
        self.protocol.initialize(stage)
        return self._collect_consequences()

    def handle_timer(self, timer: dict) -> List[_Consequence]:
        self.protocol.handle_timer(timer)
        return self._collect_consequences()

    def handle_packet(self, message: str) -> List[_Consequence]:
        self.protocol.handle_packet(message)
        return self._collect_consequences()

    def handle_telemetry(self, telemetry: Telemetry) -> List[_Consequence]:
        self.protocol.handle_telemetry(telemetry)
        return self._collect_consequences()

    def finish(self) -> List[_Consequence]:
        self.protocol.finish()
        return self._collect_consequences()
