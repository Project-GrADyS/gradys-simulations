#include "inet/common/INETDefs.h"

#ifndef __INET_UdpMobileNodeCommunicationApp_H
#define __INET_UdpMobileNodeCommunicationApp_H

#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "messages/network/MobileNodeMessage_m.h"
#include "Telemetry_m.h"
#include "UdpBasicAppMobileNode.h"

using namespace inet;

namespace projeto {

enum CommunicationStatus { FREE=0, REQUESTING=1, PAIRED=2, COLLECTING=3, PAIRED_FINISHED=4 };

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

        // Communication status variable
        CommunicationStatus communicationStatus = FREE;

        // Current target
        int tentativeTarget = -1;
        // Previous target
        int lastTarget = -1;
        // Name of the current target (for addressing purposes)
        std::string tentativeTargetName;

        // Current imaginary data being carried
        int currentDataLoad=0;
        // Stable data load to prevent data loss during pairing
        int stableDataLoad=currentDataLoad;

        // Last telemetry package recieved
        Telemetry currentTelemetry = Telemetry();
        Telemetry lastStableTelemetry = Telemetry();

    private:
        // Initiates a timeout timer
        virtual void initiateTimeout();
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

