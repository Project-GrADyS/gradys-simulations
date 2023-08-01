//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package gradys_simulations.mobility;

import gradys_simulations.mobility.base.MAVLinkMobilityBase;

// 
// This module is analogous to INET's RandomWaypointMobility. The vehicle will follow random waypoints within the environment constraints
// and wait there for a set amount before traveling to the next one. 
//
simple MAVLinkRandomWaypointMobility extends MAVLinkMobilityBase
{
    parameters:
        // Speed at which the vehicle should travel
        volatile double speed @unit(mps) = default(20mps);
        // Time the vehicle will wait before traveling to the next waypoint
        volatile double waitTime @unit(s) = default(0s); // wait time between reaching a target and choosing a new one
        // Radius of the waypoint. Vehicles within this radius will be considered to have reached the waypoint
        double waypointRadius @unit(m) = default(30m);
        @class(MAVLinkRandomWaypointMobility);
}
