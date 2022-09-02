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

namespace projeto;


enum NodeType {
    UAV=1;
    SENSOR=2;
}

enum MessageType {
    SHARE=1;
    ACK=2;
}

class CentralizedQMessage extends inet::FieldsChunk {
    chunkLength = inet::B(15); // Fixed chunk length
    NodeType nodeType; // Type of the sender node
    MessageType messageType; // Type of message
    int nodeId; // Id of the sender node
    int targetId; // Id of the target node
    NodeType targetNodeType; // Type of the target node
    int packetLoad; // Number of packets sent in message
}