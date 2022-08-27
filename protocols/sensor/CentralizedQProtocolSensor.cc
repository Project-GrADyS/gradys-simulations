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

#include "CentralizedQProtocolSensor.h"

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

namespace projeto {

Define_Module(CentralizedQProtocolSensor);

void CentralizedQProtocolSensor::initialize(int stage)
{
    CommunicationProtocolBase::initialize(stage);

    if(stage == INITSTAGE_LOCAL) {
        // Signal that carries current data load and is emitted every time it is updated
        dataLoadSignalID = registerSignal("dataLoad");
        emit(dataLoadSignalID, 0);

        learning = dynamic_cast<CentralizedQLearning*>(getModuleByPath("learner"));
        sensorId = learning->registerSensor(this);
        beta = par("beta");

        scheduleAt(simTime() + beta, generationMessage);
    }
}

void CentralizedQProtocolSensor::handleMessage(cMessage* msg) {
    if (msg->isSelfMessage()) {
        if(msg->getKind() == GENERATE) {
            awaitingPackages++;
            emit(dataLoadSignalID, awaitingPackages);
            scheduleAt(simTime() + beta, generationMessage);

            // Updating payload with new amount of awaiting packages
            CentralizedQMessage *payload = new CentralizedQMessage();
            payload->setNodeType(UAV);
            payload->setMessageType(SHARE);
            payload->setNodeId(sensorId);
            payload->setTargetId(-1);
            payload->setPacketLoad(awaitingPackages);

            CommunicationCommand *command = new CommunicationCommand();
            command->setCommandType(SET_PAYLOAD);
            command->setPayloadTemplate(payload);
            sendCommand(command);
        } else {

            scheduleAt(simTime() + messageInterval, commMessage);
        }
    }
}

void CentralizedQProtocolSensor::handlePacket(Packet *pk) {
    auto payload = dynamicPtrCast<const CentralizedQMessage>(pk->peekAtBack());
    if(payload != nullptr && (payload->getTargetId() == sensorId || payload->getTargetId() == -1)) {
        if(payload->getMessageType() == ACK) {
            awaitingPackages = 0;
            emit(dataLoadSignalID, 0);
        }
    }
}
} //namespace
