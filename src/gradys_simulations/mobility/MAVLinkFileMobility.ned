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
// Mobility module that reads a MAVLink waypoint mission file, in the same format produced by
// Mission Planner. This mission is relayed to the vehicle which follows it.
//
simple MAVLinkFileMobility extends MAVLinkMobilityBase
{
    parameters:
    	@class(MAVLinkFileMobility);
    	// Path to the waypoint mission file.
    	string waypointFile; 
}
