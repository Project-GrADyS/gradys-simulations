from enum import Enum
from typing import TypedDict


class SenderType(Enum):
    DRONE = 1
    SENSOR = 2
    GROUND_STATION = 3


class SimpleMessage(TypedDict):
    sender: SenderType
    content: int
