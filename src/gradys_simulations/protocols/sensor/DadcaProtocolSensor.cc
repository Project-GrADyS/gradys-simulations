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

#include "DadcaProtocolSensor.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "inet/applications/base/ApplicationPacket_m.h"

namespace gradys_simulations {

Define_Module(DadcaProtocolSensor);

void DadcaProtocolSensor::initialize(int stage)
{
    CommunicationProtocolBase::initialize(stage);

    if(stage == INITSTAGE_LOCAL) {
        generationInterval = par("generationInterval");
        scheduleAt(simTime() + generationInterval, generationTimer);
        dataLoadSignalID = registerSignal("dataLoad");
        emit(dataLoadSignalID, awaitingPackets);
    }
}

void DadcaProtocolSensor::handleMessage(cMessage *msg) {
    if(msg == generationTimer) {
        awaitingPackets++;
        emit(dataLoadSignalID, awaitingPackets);
        scheduleAt(simTime() + generationInterval, generationTimer);
    } else {
        CommunicationProtocolBase::handleMessage(msg);
    }
}

void DadcaProtocolSensor::handlePacket(Packet *pk) {
    auto payload = pk->peekAtBack<DadcaMessage>(B(34), 1);


    if(payload != nullptr) {
        if(payload->getMessageType() == DadcaMessageType::HEARTBEAT)
        {
            EV_INFO << this->getParentModule()->getFullName() << " recieved heartbeat from " << tentativeTarget << endl;
            tentativeTarget = payload->getSourceID();
            tentativeTargetName = pk->getName();
            updatePayload(tentativeTargetName.c_str());
        }
    }
}

void DadcaProtocolSensor::updatePayload(const char* target) {
    DadcaMessage *payload = new DadcaMessage();
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());

    payload->setMessageType(DadcaMessageType::BEARER);
    payload->setSourceID(this->getParentModule()->getId());
    payload->setDestinationID(tentativeTarget);
    payload->setDataLength(awaitingPackets);
    EV_INFO << payload->getSourceID() << " sending bearer to " << tentativeTarget  << endl;

    lastPayload = *payload;

    CommunicationCommand *command = new CommunicationCommand();
    command->setCommandType(SEND_MESSAGE);
    command->setPayloadTemplate(payload);
    command->setTarget(target);
    sendCommand(command);
    awaitingPackets = 0;
    emit(dataLoadSignalID, 0);
}

} //namespace
