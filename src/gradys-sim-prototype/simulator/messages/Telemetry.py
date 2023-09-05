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

    def __init__(
        self,
        next_waypoint_id=0,
        last_waypoint_id=0,
        current_lat=0,
        current_lon=0,
        current_alt=0,
        is_reversed=False,
        drone_activity=DroneActivity.IDLE,
    ):
        self.next_waypoint_id = next_waypoint_id
        self.last_waypoint_id = last_waypoint_id
        self.current_lat = current_lat
        self.current_lon = current_lon
        self.current_alt = current_alt
        self.is_reversed = is_reversed
        self.drone_activity: drone_activity
