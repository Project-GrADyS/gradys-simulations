#include "inet/common/INETDefs.h"

#ifndef __INET_UdpMobileNodeCommunicationApp_H
#define __INET_UdpMobileNodeCommunicationApp_H

#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "MobileNodeMessage_m.h"
#include "Telemetry_m.h"
#include "UdpBasicAppMobileNode.h"

using namespace inet;

namespace projeto {

class UdpMobileNodeCommunicationApp : public UdpBasicAppMobileNode
{
    private:
        // Indicates the time when the last timeout started
        simtime_t timeoutStart;
        // Timeout status
        bool isTimedout = false;
        // True if the drone has recieved a heartbeat but not a pair request
        bool isRequested = false;
        // True if the drone's last interaction has concluded
        bool isDone = false;
        // Current target
        int tentativeTarget = -1;
        // Previous target
        int lastTarget = -1;
        // Name of the current target (for addressing purposes)
        std::string tentativeTargetName;

        // Capacity to carry imaginary data
        int dataCapacity;
        // Current imaginary data being carried
        int currentDataLoad=0;

        // Last telemetry package recieved
        Telemetry currentTelemetry = Telemetry();
        Telemetry lastStableTelemetry = Telemetry();

    private:
        // Resets parameters after interaction has concluded or cancelled
        virtual void resetParameters();
        // Sends a reverse order to the mobility module
        virtual void sendReverseOrder();

    protected:
        // NED variable for the duration of the communication timeout
        double timeoutDuration;

    protected:
        virtual void initialize(int stage) override;
        virtual void setSocketOptions() override;
        virtual void sendPacket() override;
        virtual void processPacket(Packet *pk) override;
        virtual void handleMessageWhenUp(cMessage *msg) override;

        virtual void sendHeartbeat(inet::IntrusivePtr<inet::MobileNodeMessage> payload);
        virtual void sendPairRequest(inet::IntrusivePtr<inet::MobileNodeMessage> payload, int target);
        virtual void sendPairConfirm(inet::IntrusivePtr<inet::MobileNodeMessage> payload, int target);

        // Checks and updates the current timeout status (true if the drone is FREE)
        virtual bool checkAndUpdateTimeout();
    public:
        simsignal_t dataLoadSignalID;
};

} // namespace inet

#endif // ifndef __INET_UdpBasicAppMobileNode_H

