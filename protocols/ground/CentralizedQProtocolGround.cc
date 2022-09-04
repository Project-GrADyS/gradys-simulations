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
#include "../../applications/mamapp/BMeshPacket_m.h"
#include "../messages/internal/CommunicationCommand_m.h"
#include "../messages/internal/MobilityCommand_m.h"
#include <numeric>
#include "CentralizedQProtocolGround.h"

namespace projeto {

Define_Module(CentralizedQProtocolGround);

void CentralizedQProtocolGround::initialize(int stage)
{
    CommunicationProtocolBase::initialize(stage);

    if(stage == INITSTAGE_LOCAL) {
        // Signal that carries current data load and is emitted every time it is updated
        dataLoadSignalID = registerSignal("dataLoad");
        emit(dataLoadSignalID, 0);

        messageInterval = par("messageInterval");

        scheduleAt(simTime() + messageInterval, receiveMessage);
    }
}

void CentralizedQProtocolGround::handleMessage(cMessage* msg) {
    if (msg->isSelfMessage()) {
        if(msg == receiveMessage) {
            // Trying to send the currently awaiting packages to any listening sensor
            CentralizedQMessage *payload = new CentralizedQMessage();
            payload->setNodeType(GROUND);
            payload->setMessageType(RECEIVE);
            payload->setNodeId(0);
            payload->setTargetId(-1);
            payload->setTargetNodeType(UAV);

            CommunicationCommand *command = new CommunicationCommand();
            command->setCommandType(SEND_MESSAGE);
            command->setPayloadTemplate(payload);
            command->setTarget(nullptr);
            sendCommand(command);

            scheduleAt(simTime() + messageInterval, receiveMessage);
            return;
        }
    }
    CommunicationProtocolBase::handleMessage(msg);
}

void CentralizedQProtocolGround::handlePacket(Packet *pk) {
    auto payload = dynamicPtrCast<const CentralizedQMessage>(pk->peekAtBack());
    if(payload != nullptr && (payload->getTargetId() == 0 || payload->getTargetId() == -1)  && payload->getTargetNodeType() == GROUND) {
        if(payload->getMessageType() == SHARE) {
            receivedPackets += payload->getPacketLoad();
            emit(dataLoadSignalID, receivedPackets);

            CentralizedQMessage *response = new CentralizedQMessage();
            response->setNodeType(GROUND);
            response->setMessageType(ACK);
            response->setNodeId(0);
            response->setTargetId(payload->getNodeId());
            response->setTargetNodeType(UAV);

            CommunicationCommand *command = new CommunicationCommand();
            command->setCommandType(SEND_MESSAGE);
            command->setPayloadTemplate(response);
            command->setTarget(nullptr);
            sendCommand(command);
        }
    }
}
} //namespace
