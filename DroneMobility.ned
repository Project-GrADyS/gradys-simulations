package projeto;

import inet.mobility.single.VehicleMobility;

simple GeoWaypointMobility extends VehicleMobility
{
    parameters:
        @class(GeoWaypointMobility);
        double climbAngle @unit(radian) = default(45deg);
}

