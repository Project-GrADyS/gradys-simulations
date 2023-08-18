from enum import Enum


class DroneActivity(Enum):
    IDLE = 0
    NAVIGATING = 1
    REACHED_EDGE = 2
    FOLLOWING_COMMAND = 3


class Telemetry:
    next_waypoint_id: int
    last_waypoint_id: int

    current_lat: float
    current_lon: float
    current_alt: float

    is_reversed: bool

    drone_activity: DroneActivity
