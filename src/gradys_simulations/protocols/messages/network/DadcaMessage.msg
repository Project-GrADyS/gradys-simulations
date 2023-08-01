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

import inet.common.packet.chunk.Chunk;

namespace inet;


enum DadcaMessageType
{
  HEARTBEAT = 0; 
  PAIR_REQUEST = 1; 
  PAIR_CONFIRM = 2;
  BEARER = 3;
}

class DadcaMessage extends FieldsChunk
{
  chunkLength = B(34); // Fixed chunk length
  int sourceID = -1;  // ID of the message's source
  int destinationID = -1; // ID of the message's destination
  int nextWaypointID = -1; // ID of the next waypoint
  int lastWaypointID = -1; // ID of the last waypoint
  int dataLength = 5; // Length of the imaginary data being carried in the message
  int leftNeighbours = 0; // Neighbours to the left of the drone
  int rightNeighbours = 0; // Neighbours to the right of the drone
  bool reversed = false; // Reverse flag which indicates the current direction the drone is travelling in
  DadcaMessageType messageType = HEARTBEAT; // Type of message
}