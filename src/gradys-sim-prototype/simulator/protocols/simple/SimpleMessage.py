from enum import Enum
from typing import TypedDict


class SenderType(Enum):
    DRONE = 0
    SENSOR = 1
    GROUND_STATION = 2

class SimpleMessage(TypedDict):
    sender: SenderType
    content: int