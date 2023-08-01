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

// Activity that the drone is currently carrying out
enum DroneActivity { 
    IDLE=0; 
    NAVIGATING=1;
    REACHED_EDGE=2; 
    FOLLOWING_COMMAND=3;
    RECHARGING=4;
    SHUTDOWN=5;
}

// Message declaration designed to share necessary drone information with the communication module
message Telemetry {
    int nextWaypointID=-1;
    int lastWaypointID=-1;
    int currentCommand=-1;
    double currentX;
    double currentY;
    double currentZ;
    double currentLat;
    double currentLon;
    double currentAlt;
    bool isReversed=false;
    DroneActivity droneActivity;
}
