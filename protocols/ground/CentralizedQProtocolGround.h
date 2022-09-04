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

#ifndef __PROJETO_CentralizedQProtocolGround_H_
#define __PROJETO_CentralizedQProtocolGround_H_

#include <omnetpp.h>
#include "../base/CommunicationProtocolBase.h"
#include "../auxiliary/CentralizedQLearning.h"
#include "../messages/internal/Telemetry_m.h"
#include "../messages/network/CentralizedQMessage_m.h"
#include "inet/common/geometry/common/Coord.h"

using namespace omnetpp;

namespace projeto {

class CentralizedQProtocolGround : public CommunicationProtocolBase
{
protected:
    int receivedPackets = 0;

    simtime_t messageInterval;

    cMessage *receiveMessage = new cMessage(nullptr);

protected:
    virtual void initialize(int stage) override;

    virtual void handleMessage(cMessage *msg) override;

    // Reacts to message recieved and updates payload accordingly
    virtual void handlePacket(Packet *pk) override;
public:
    simsignal_t dataLoadSignalID;
};

} //namespace

#endif
