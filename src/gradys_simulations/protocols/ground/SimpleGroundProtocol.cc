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

#include "gradys_simulations/protocols/ground/SimpleGroundProtocol.h"

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
Define_Module(SimpleGroundProtocol);

void SimpleGroundProtocol::initialize(int stage) {
    emit(registerSignal("dataLoad"), content);

    // Sets the correct payload
    SimpleMessage *payload = new SimpleMessage();
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    payload->setSenderType(GROUND_STATION);
    payload->setContent(0);

    CommunicationCommand *command = new CommunicationCommand();
    command->setCommandType(CommunicationCommandType::SET_PAYLOAD);
    command->setPayloadTemplate(payload);
    sendCommand(command);
}

void SimpleGroundProtocol::handlePacket(Packet *pk) {
    auto message = pk->peekAtBack<SimpleMessage>(B(7), 1);

    if(message != nullptr) {
        switch(message->getSenderType()) {
            case DRONE:
            {
                content += message->getContent();
                // Emits signal on data content change
                emit(registerSignal("dataLoad"), content);

                // Sets the correct target
                CommunicationCommand *targetCommand = new CommunicationCommand();
                targetCommand->setCommandType(CommunicationCommandType::SEND_MESSAGE);
                targetCommand->setTarget(pk->getName());
                sendCommand(targetCommand);
                break;
            }
            case SENSOR:
            {
                break;
            }
            case GROUND_STATION:
            {
                break;
            }
        }
    }
} /* namespace gradys_simulations */

}
