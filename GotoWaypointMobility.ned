package projeto;

import inet.mobility.single.RandomWaypointMobility;

simple GotoWaypointMobility extends RandomWaypointMobility
{
    parameters:
        double initialXgoTo = default(0);
        double initialYgoTo = default(0);
        double initialZgoTo = default(0);
        @class(GotoWaypointMobility);
}

