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

#ifndef __gradys_simulations_CentralizedQProtocolGround_H_
#define __gradys_simulations_CentralizedQProtocolGround_H_

#include <omnetpp.h>
#include "gradys_simulations/protocols/base/CommunicationProtocolBase.h"
#include "gradys_simulations/protocols/auxiliary/CentralizedQLearning.h"
#include "gradys_simulations/protocols/messages/internal/Telemetry_m.h"
#include "gradys_simulations/protocols/messages/network/CentralizedQMessage_m.h"
#include "inet/common/geometry/common/Coord.h"

using namespace omnetpp;

namespace gradys_simulations {

class CentralizedQProtocolGround : public CommunicationProtocolBase, public CentralizedQLearning::CentralizedQGround
{
public:
    uint32_t getReceivedPackets() override { return receivedPackets; }
protected:
    // Packets collected from agents stored in the ground station
    uint32_t receivedPackets = 0;

    simtime_t dataLoggingInterval;
    cMessage* dataLoggingTimer = new cMessage();

protected:
    // OMNET++ methods
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

    // Reacts to messages received from the commuication module
    virtual void handlePacket(Packet *pk) override;
public:
    simsignal_t dataLoadSignalID;
    simsignal_t throughputSignalID;
};

} //namespace

#endif
