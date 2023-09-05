from enum import Enum

class CommunicationCommandType(Enum):
    SEND = 1
    BROADCAST = 2


class CommunicationCommand:
    command: CommunicationCommandType
    message: dict


class SendMessageCommand(CommunicationCommand):
    def __init__(self, message: dict):
        self.command = CommunicationCommandType.SEND.name
        self.message = message


class BroadcastMessageCommand(CommunicationCommand):
    def __init__(self, message: dict):
        self.command = CommunicationCommandType.BROADCAST.name
        self.message = message
