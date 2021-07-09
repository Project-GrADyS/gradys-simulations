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


enum MessageType
{
  HEARTBEAT = 0; 
  PAIR_REQUEST = 1; 
  PAIR_CONFIRM = 2; 
}

class MobileNodeMessage extends FieldsChunk
{
  chunkLength = B(9);
  int sourceID = -1; 
  int destinationID = -1; 
  MessageType messageType = HEARTBEAT;
}