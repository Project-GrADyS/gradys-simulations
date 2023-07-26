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

#include "inet/common/ModuleAccess.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "inet/applications/base/ApplicationPacket_m.h"
#include "gradys_simulations/applications/mamapp/BMeshPacket_m.h"
#include "gradys_simulations/protocols/messages/internal/CommunicationCommand_m.h"
#include "gradys_simulations/protocols/messages/internal/MobilityCommand_m.h"
#include <numeric>
#include "CentralizedQProtocolGround.h"

namespace gradys_simulations {

Define_Module(CentralizedQProtocolGround);

void CentralizedQProtocolGround::initialize(int stage)
{
    CommunicationProtocolBase::initialize(stage);

    if(stage == INITSTAGE_LOCAL) {
        // Signal that carries current data load and is emitted every time it is updated
        dataLoadSignalID = registerSignal("dataLoad");
        throughputSignalID = registerSignal("throughput");

        dataLoggingInterval = par("dataLoggingInterval");

        CentralizedQLearning* learning = dynamic_cast<CentralizedQLearning*>(getModuleByPath("learner"));
        learning->registerGround(this);

        scheduleAt(simTime() + dataLoggingInterval, dataLoggingTimer);

        emit(dataLoadSignalID, 0);
        emit(throughputSignalID, 0.);
    }
}

void CentralizedQProtocolGround::handleMessage(cMessage *msg) {
    if(msg == dataLoggingTimer) {
        emit(throughputSignalID, static_cast<double>(receivedPackets) / simTime());
        scheduleAt(simTime() + dataLoggingInterval, dataLoggingTimer);
    } else {
        CommunicationProtocolBase::handleMessage(msg);
    }
}

void CentralizedQProtocolGround::handlePacket(Packet *pk) {
    auto payload = dynamicPtrCast<const CentralizedQMessage>(pk->peekAtBack(B(15), 1));
    // Ignoring messages not destined for a passive node
    if(payload != nullptr && (payload->getTargetNodeType() == PASSIVE || payload->getTargetNodeType() == ALL)) {
        switch(payload->getMessageType()) {
            // The ground station is constantly waiting for agents to be nearby so it can request
            // the packets they are carrying. When it hears a REQUEST message from one, it sends
            // a request message back.
            case REQUEST:
            {
                CentralizedQMessage *response = new CentralizedQMessage();
                response->setNodeType(PASSIVE);
                response->setMessageType(REQUEST);
                response->setNodeId(0);
                response->setTargetId(payload->getNodeId());
                response->setTargetNodeType(payload->getNodeType());

                CommunicationCommand *command = new CommunicationCommand();
                command->setCommandType(SEND_MESSAGE);
                command->setPayloadTemplate(response);
                command->setTarget(nullptr);
                sendCommand(command);
                break;
            }
            // When an agent shares it's packet load to the ground station it collet's and saves the data
            // and responds with an acknowledgment so the agent can discard the sent packets.
            case SHARE:
            {
                receivedPackets += payload->getPacketLoad();
                emit(dataLoadSignalID, static_cast<long>(receivedPackets));
                break;
            }
            default:
            {
                return;
            }
        }
    }
}

void CentralizedQProtocolGround::finish() {
    CommunicationProtocolBase::finish();
    cancelAndDelete(dataLoggingTimer);
}

} //namespace
