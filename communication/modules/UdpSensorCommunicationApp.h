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

#ifndef COMMUNICATION_UDPSENSORCOMMUNICATIONAPP_H_
#define COMMUNICATION_UDPSENSORCOMMUNICATIONAPP_H_

#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/common/packet/chunk/Chunk_m.h"
#include "base/UdpBasicAppMobileSensorNode.h"

namespace projeto {

class UdpSensorCommunicationApp: public UdpBasicAppMobileSensorNode {
    protected:
        char *targetName = nullptr;
        FieldsChunk *payloadTemplate = nullptr;
    protected:
        virtual void setSocketOptions() override;
        virtual void sendPacket() override { return; };
        virtual void sendPacket(char *target);
        virtual void handleMessageWhenUp(cMessage *msg) override;
        virtual void processPacket(Packet *pk) override;
};

} /* namespace projeto */

#endif /* COMMUNICATION_UDPSENSORCOMMUNICATIONAPP_H_ */
