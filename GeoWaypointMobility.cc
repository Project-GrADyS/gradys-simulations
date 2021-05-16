#include "GeoWaypointMobility.h"

using namespace inet;


namespace projeto {

Define_Module(GeoWaypointMobility);

GeoWaypointMobility::GeoWaypointMobility() {}

void GeoWaypointMobility::move () {

    // Identical to VehibleMobility, except direction change is instant
    // upon reaching a waypoint
    Waypoint target = waypoints[targetPointIndex];
    double dx = target.x - lastPosition.x;
    double dy = target.y - lastPosition.y;
    if (dx * dx + dy * dy < waypointProximity * waypointProximity)  // reached so change to next (within the predefined proximity of the waypoint)
    {
        targetPointIndex = (targetPointIndex + 1) % waypoints.size();

        // Always facing towards target
        heading = atan2(dy, dx) / M_PI * 180;

        tempSpeed = Coord(cos(M_PI * heading / 180), sin(M_PI * heading / 180)) * speed;
    }

    double timeStep = (simTime() - lastUpdate).dbl();
    Coord tempPosition = lastPosition + tempSpeed * timeStep;

    if (ground)
        tempPosition = ground->computeGroundProjection(tempPosition);

    lastVelocity = tempPosition - lastPosition;
    lastPosition = tempPosition;
}

}
