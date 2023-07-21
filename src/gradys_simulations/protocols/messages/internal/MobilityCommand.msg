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

namespace gradys_simulations;

// Commands that the mobility module should be capable of carrying out
enum MobilityCommandType {
    // Makes the drone reverse on its course
    // No params
    REVERSE=0; 
    
    // Makes the drone travel to a specific waypoint, following the tour pack
    // Param 1: Waypoint index
    GOTO_WAYPOINT=1;
    
    // Makes the drone go to a specific coordinate and orient itself so it can continue the tour afterwards
    // Param 1: x component of the coord
    // Param 2: y component of the coord
    // Param 3: z component of the coord
    // Param 4: Next waypoint (Waypoint the drone should go to after reaching the target)
    // Param 5: Last waypoint (Waypoint the drone used to reach the coords)
    GOTO_COORDS=2;
    
    // Orders the drone to return to the home coordinates
    // No params
    RETURN_TO_HOME=3;
    
    // Idles the drone for a specified time
    // Param 1: idle time in seconds
    IDLE_TIME = 4;
    
    // Simulates vehicle recharging. Basically a combination of the RETURN_TO_HOME and IDLE_TIME commands
    // Param 1: Time to recharge
    RECHARGE = 5;
    
    // Shuts down the drone, making it land and permanently stay idle
    // This command is special as it takes priority over other commands
    // No params
    FORCE_SHUTDOWN = 6;
    
    // Cancels an active FORCE_SHUTDOWN command
    // No params
    WAKE_UP = 7;
}

// Message declaration containing the command Id and its parameters 
message MobilityCommand {
    MobilityCommandType commandType;
    double param1=-1;
    double param2=-1;
    double param3=-1;
    double param4=-1;
    double param5=-1;
}
