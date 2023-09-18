from enum import Enum
import json

class SenderType(int, Enum):
    DRONE = 0
    SENSOR = 1
    GROUND_STATION = 2

class SimpleMessage:
    sender: SenderType
    content: int

    def __init__(self, sender: SenderType, content: int) -> None:
        self.sender = sender
        self.content = content

    def to_json(self):
        return json.dumps(self.__dict__)

    @classmethod
    def from_json(cls, json_str):
        data = json.loads(json_str)
        return cls(**data)