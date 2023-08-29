from enum import Enum


class MobilityCommandType(Enum):
    SET_MODE = 1
    GOTO_COORDS = 2

    GOTO_WAYPOINT = 3
    REVERSE = 4


class MobilityMode(Enum):
    GUIDED = 1
    AUTO = 2


class MobilityCommand:
    command: MobilityCommandType

    param_1: int
    param_2: int
    param_3: int
    param_4: int
    param_5: int
    param_6: int


class ReverseCommand(MobilityCommand):
    def __init__(self):
        self.command = MobilityCommandType.REVERSE.name

        self.param_1 = 0
        self.param_2 = 0
        self.param_3 = 0
        self.param_4 = 0
        self.param_5 = 0
        self.param_6 = 0


class SetModeCommand(MobilityCommand):
    def __init__(self, mode: MobilityMode):
        self.command = MobilityCommandType.SET_MODE.name
        self.param_1 = mode.value

        self.param_2 = 0
        self.param_3 = 0
        self.param_4 = 0
        self.param_5 = 0
        self.param_6 = 0
