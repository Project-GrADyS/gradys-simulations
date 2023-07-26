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

namespace gradys_simulations {

Define_Module(CentralizedQProtocolSensor);

void CentralizedQProtocolSensor::initialize(int stage)
{
    CommunicationProtocolBase::initialize(stage);

    if(stage == INITSTAGE_LOCAL) {
        // Signal that carries current data load and is emitted every time it is updated
        dataLoadSignalID = registerSignal("dataLoad");

        maxAwaitingPackets = par("maxAwaitingPackets");
        awaitingPackets = maxAwaitingPackets;
        emit(dataLoadSignalID, static_cast<long>(awaitingPackets));

        learning = dynamic_cast<CentralizedQLearning*>(getModuleByPath("learner"));
        sensorId = learning->registerSensor(this);
        sensorPosition = par("sensorPosition");
        beta = par("beta");

        // Scheduling packet generation timer
        scheduleAt(simTime() + beta, generationTimer);
    }
}

void CentralizedQProtocolSensor::finish() {
    cancelAndDelete(generationTimer);
}

void CentralizedQProtocolSensor::handleMessage(cMessage* msg) {
    if (msg->isSelfMessage()) {
        // This means the timer has triggered and a new packet should be generated
        if(msg == generationTimer) {
            // Generating a new package and storing it with the awaited packages
            awaitingPackets++;
            if (awaitingPackets > maxAwaitingPackets) {
                awaitingPackets = maxAwaitingPackets;
            }
            emit(dataLoadSignalID, static_cast<long>(awaitingPackets));
            scheduleAt(simTime() + beta, generationTimer);
            return;
        }
    }
    CommunicationProtocolBase::handleMessage(msg);
}

void CentralizedQProtocolSensor::handlePacket(Packet *pk) {
    auto payload = dynamicPtrCast<const CentralizedQMessage>(pk->peekAtBack(B(15), 1));
    // Ignores messages not destined for this sensor id (or -1)
    if(payload != nullptr && (payload->getTargetId() == sensorId || payload->getTargetId() == -1)
            && (payload->getTargetNodeType() == PASSIVE || payload->getTargetNodeType() == ALL)) {
        switch(payload->getMessageType()) {
            // When a sensor receives a REQUEST messages it sends all it's awaiting packets to the sender
            case REQUEST:
            {
                // No point in sharing packets if you have none
                if(awaitingPackets == 0) {
                    break;
                }

                CentralizedQMessage *response = new CentralizedQMessage();
                response->setNodeType(PASSIVE);
                response->setNodeId(sensorId);
                response->setMessageType(SHARE);
                response->setTargetId(payload->getNodeId());
                response->setTargetNodeType(payload->getNodeType());
                response->setPacketLoad(awaitingPackets);

                CommunicationCommand *command = new CommunicationCommand();
                command->setCommandType(SEND_MESSAGE);
                command->setPayloadTemplate(response);
                command->setTarget(nullptr);
                sendCommand(command);

                awaitingPackets = 0;
                visited = true;
                emit(dataLoadSignalID, 0);
                break;
            }
            // When a sensor receives an ACK message it means that the packets that it's shared have reached
            // an agent and it is acknowledging that fact. In that case the sensor discards the currently
            // awaiting packets.
            case ACK:
            {
                awaitingPackets = 0;
                visited = true;
                emit(dataLoadSignalID, 0);
                break;
            }
            case SHARE:
                break;
        }
    }
}

} //namespace
