from enum import Enum

from simulator.protocols.simple.SimpleMessage import SimpleMessage


class CommunicationCommandType(Enum):
    SEND = 1
    BROADCAST = 2


class CommunicationCommand:
    command: CommunicationCommandType
    message: SimpleMessage


class SendMessageCommand(CommunicationCommand):
    def __init__(self, message: SimpleMessage):
        self.command = CommunicationCommandType.SEND.name
        self.message = message


class BroadcastMessageCommand(CommunicationCommand):
    def __init__(self, message: SimpleMessage):
        self.command = CommunicationCommandType.BROADCAST.name
        self.message = message
