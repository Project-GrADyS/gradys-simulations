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

#include "ZigzagProtocolSensor.h"

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

Define_Module(ZigzagProtocolSensor);

void ZigzagProtocolSensor::initialize(int stage)
{
    CommunicationProtocolBase::initialize(stage);
}

void ZigzagProtocolSensor::handlePacket(Packet *pk) {
    auto payload = pk->peekAtBack<ZigzagMessage>(B(14), 1);

    if(payload != nullptr) {
        if(payload->getMessageType() == ZigzagMessageType::HEARTBEAT)
        {
            tentativeTarget = payload->getSourceID();
            tentativeTargetName = pk->getName();
            std::cout << this->getParentModule()->getFullName() << " recieved heartbeat from " << tentativeTarget << endl;
            setTarget(tentativeTargetName.c_str());
            updatePayload();
        }
    }
}

void ZigzagProtocolSensor::updatePayload() {
    ZigzagMessage *payload = new ZigzagMessage();
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());

    payload->setMessageType(ZigzagMessageType::BEARER);
    payload->setSourceID(this->getParentModule()->getId());
    payload->setDestinationID(tentativeTarget);
    std::cout << payload->getSourceID() << " sending bearer to " << tentativeTarget  << endl;

    CommunicationCommand *command = new CommunicationCommand();
    command->setCommandType(SET_PAYLOAD);
    command->setPayloadTemplate(payload);
    sendCommand(command);
}

void ZigzagProtocolSensor::setTarget(const char *target) {
    CommunicationCommand *command = new CommunicationCommand();
    command->setCommandType(SET_TARGET);
    command->setTarget(target);
    sendCommand(command);
}
} //namespace
