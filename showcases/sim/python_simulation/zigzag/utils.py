from enum import Enum


class CommunicationStatus(Enum):
    FREE = 0
    REQUESTING = 1
    PAIRED = 2
    PAIRED_FINISHED = 3
