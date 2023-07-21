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

namespace gradys_simulations;

enum CommunicationCommandType {
    // Sets the payload that the communication module sends
    SET_PAYLOAD=0;
    // Sets the target of the communication message (null means broadcast)
    SET_TARGET=1;
    // Sends a one time message with the payload AND target specified
    SEND_MESSAGE=2;
}

// Message declaration for the communication command
message CommunicationCommand {
    // Command Id
    CommunicationCommandType commandType;
    
    // Template for the SET_PAYLOAD message type (message that the communication module should send)
    inet::FieldsChunk *payloadTemplate = nullptr;
    
    // Target for the set target command
    string target;
}
