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

#include "SimpleDroneProtocol.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/packet/Packet.h"

namespace gradys_simulations {
Define_Module(SimpleDroneProtocol);

void SimpleDroneProtocol::initialize(int stage) {
    // Loading the parameter timeoutDuration
    timeoutDuration = par("timeoutDuration");

    // Emits the first dataLoad signal with value 0
    emit(registerSignal("dataLoad"), content);

    WATCH(content);
    WATCH_MAP(contentSources);
    // Updates the payload so the drone can start sending messages
    updatePayload();
}

void SimpleDroneProtocol::handlePacket(Packet *pk) {
    // Loads the SimpleMessage from the received packet
    auto message = pk->peekAtBack<SimpleMessage>(B(7), 1);

    if(message != nullptr) {
        switch(message->getSenderType()) {
            case DRONE:
            {
                std::cout << "Message recieved from drone, ignoring." << endl;
                break;
            }
            case SENSOR:
            {
                if(!isTimedout()) {
                    content += message->getContent();

                    contentSources[std::string(pk->getName())]++;

                    // Emits signal and updates payload on data content change
                    emit(registerSignal("dataLoad"), content);
                    updatePayload();

                    initiateTimeout(timeoutDuration);
                }
                break;
            }
            case GROUND_STATION:
            {
                if(content != 0 && !isTimedout()) {
                    content = 0;
                    // Emits signal and updates payload on data content change
                    emit(registerSignal("dataLoad"), content);
                    updatePayload();

                    initiateTimeout(timeoutDuration);
                }
                break;
            }
        }
    }
}

void SimpleDroneProtocol::updatePayload() {
    // Creates message template with current content and correct type
    SimpleMessage *payload = new SimpleMessage();
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    payload->setSenderType(DRONE);
    payload->setContent(content);

    // Sends command to the communication module to start using this message
    CommunicationCommand *command = new CommunicationCommand();
    command->setCommandType(CommunicationCommandType::SET_PAYLOAD);
    command->setPayloadTemplate(payload);
    sendCommand(command);
}

void SimpleDroneProtocol::finish() {
    CommunicationProtocolBase::finish();

    for(auto const& item : contentSources) {
        recordScalar(item.first.c_str(), item.second);
    }
}
} /* namespace gradys_simulations */
