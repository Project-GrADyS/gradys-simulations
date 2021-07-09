#include "inet/common/INETDefs.h"

#ifndef __INET_UdpMobileNodeCommunicationApp_H
#define __INET_UdpMobileNodeCommunicationApp_H

#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "MobileNodeMessage_m.h"
#include "../UdpBasicAppMobileNode.h"

using namespace inet;

namespace projeto {

class UdpMobileNodeCommunicationApp : public UdpBasicAppMobileNode
{
    private:
        simtime_t timeoutStart;
        bool isTimedout = false;
        bool isRequested = false;
        bool isDone = false;
        int tentativeTarget = -1;
        int lastTarget = -1;
        std::string tentativeTargetName;

    private:
        virtual void resetParameters();

    protected:
        double timeoutDuration;

    protected:
        virtual void initialize(int stage) override;
        virtual void sendPacket() override;
        virtual void processPacket(Packet *pk) override;
        virtual void processSend() override;

        virtual void sendHeartbeat(inet::IntrusivePtr<inet::MobileNodeMessage> payload);
        virtual void sendPairRequest(inet::IntrusivePtr<inet::MobileNodeMessage> payload, int target);
        virtual void sendPairConfirm(inet::IntrusivePtr<inet::MobileNodeMessage> payload, int target);

        virtual bool checkAndUpdateTimeout();
    public:
        simsignal_t pairedSignalId;
};

} // namespace inet

#endif // ifndef __INET_UdpBasicAppMobileNode_H

