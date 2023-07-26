//
// Copyright (C) 2004 Andras Varga
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#ifndef __INET_MAMDATACOLLECTORAPP_H
#define __INET_MAMDATACOLLECTORAPP_H

#include "inet/common/INETDefs.h"

#include "inet/applications/base/ApplicationBase.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"

using namespace inet;

namespace gradys_simulations {

/**
 * Consumes and prints packets received from the Udp module. See NED for more info.
 */
class MamDataCollectorApp : public ApplicationBase, public UdpSocket::ICallback
{
  protected:
    enum SelfMsgKinds { START = 1, STOP = 2, DISCOVERY = 3 };

    UdpSocket socket;
    int localPort = -1, destPort = -1;
    L3Address multicastGroup;
    simtime_t startTime;
    simtime_t stopTime;
    bool dontFragment = false;

    cMessage *selfMsg = nullptr;
    int numSentDiscovery = 0;
    int numSentDataAck = 0;
    int numReceived = 0;
    int numUnique = 0;
    int numSenders = 0;

    long uniqueDataBytesReceived = 0;
    long excessDataBytesReceived = 0;
    long excessDataPacketsReceived = 0;

    std::set<std::string> uniqueDataSendPacketHashes;
    std::set<std::string> uniqueDataSenders;

    simsignal_t dataDelaySignal = registerSignal("dataDelay");

    simsignal_t uniqueDataCollectedSignal = registerSignal("dataLoad");

  public:
    MamDataCollectorApp() {}
    virtual ~MamDataCollectorApp();

  protected:
    virtual void processPacket(Packet *msg);
    virtual void setSocketOptions();

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void finish() override;
    virtual void refreshDisplay() const override;

    virtual void socketDataArrived(UdpSocket *socket, Packet *packet) override;
    virtual void socketErrorArrived(UdpSocket *socket, Indication *indication) override;
    virtual void socketClosed(UdpSocket *socket) override;

    virtual void processStart();
    virtual void processStop();
    virtual void processDiscovery();

    virtual void handleStartOperation(LifecycleOperation *operation) override;
    virtual void handleStopOperation(LifecycleOperation *operation) override;
    virtual void handleCrashOperation(LifecycleOperation *operation) override;

    virtual void sendDiscoveryPacket();
    virtual void sendDataAckPacket();
};

} // namespace inet

#endif // ifndef __INET_MAMDATACOLLECTORAPP_H

