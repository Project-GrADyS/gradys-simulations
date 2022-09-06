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
        if(msg == generationMessage) {
            // Generating a new package and storing it with the awaited packages
            awaitingPackages++;
            emit(dataLoadSignalID, awaitingPackages);
            scheduleAt(simTime() + beta, generationMessage);
            return;
        }
    }
    CommunicationProtocolBase::handleMessage(msg);
}

void CentralizedQProtocolSensor::handlePacket(Packet *pk) {
    auto payload = dynamicPtrCast<const CentralizedQMessage>(pk->peekAtBack());
    if(payload != nullptr && (payload->getTargetId() == sensorId || payload->getTargetId() == -1)  && payload->getTargetNodeType() == PASSIVE) {
        switch(payload->getMessageType()) {
            case REQUEST:
            {
                // Trying to send the currently awaiting packages to any listening sensor
                CentralizedQMessage *response = new CentralizedQMessage();
                response->setNodeType(PASSIVE);
                response->setNodeId(sensorId);
                response->setMessageType(SHARE);
                response->setTargetId(payload->getNodeId());
                response->setTargetNodeType(payload->getNodeType());
                response->setPacketLoad(awaitingPackages);

                CommunicationCommand *command = new CommunicationCommand();
                command->setCommandType(SEND_MESSAGE);
                command->setPayloadTemplate(response);
                command->setTarget(nullptr);
                sendCommand(command);
                break;
            }
            case ACK:
            {
                awaitingPackages = 0;
                emit(dataLoadSignalID, 0);
                break;
            }
            default:
            {
                return;
            }
        }
    }
}

} //namespace
