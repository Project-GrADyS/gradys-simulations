#include "inet/common/INETDefs.h"

#ifndef __INET_UdpMobileNodeCommunicationApp_H
#define __INET_UdpMobileNodeCommunicationApp_H

#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/common/packet/chunk/Chunk_m.h"
#include "base/UdpBasicAppMobileNode.h"

using namespace inet;

namespace projeto {

enum CommunicationStatus { FREE=0, REQUESTING=1, PAIRED=2, COLLECTING=3, PAIRED_FINISHED=4 };

class UdpMobileNodeCommunicationApp : public UdpBasicAppMobileNode
{
    protected:
        char *targetName = nullptr;
        FieldsChunk *payloadTemplate = nullptr;


    protected:
        virtual void initialize(int stage) override;
        virtual void setSocketOptions() override;
        virtual void sendPacket() override;
        virtual void processPacket(Packet *pk) override;
        virtual void handleMessageWhenUp(cMessage *msg) override;
};

} // namespace inet

#endif // ifndef __INET_UdpBasicAppMobileNode_H

