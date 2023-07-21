//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004,2011 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "UdpCommunicationApp.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "inet/applications/base/ApplicationPacket_m.h"

#include "gradys_simulations/MobileNode.h"
#include "gradys_simulations/protocols/messages/internal/CommunicationCommand_m.h"

using namespace inet;

namespace gradys_simulations {

Define_Module(UdpCommunicationApp);

void UdpCommunicationApp::initialize(int stage) {
    UdpBasicApp::initialize(stage);
}

void UdpCommunicationApp::setSocketOptions() {
    UdpBasicApp::setSocketOptions();
    // Joining multicast group used to communicate multicast messages to other drones
    socket.joinMulticastGroup(Ipv4Address("224.0.0.9"));
}

void UdpCommunicationApp::handleMessageWhenUp(cMessage *msg) {
    CommunicationCommand *command = dynamic_cast<CommunicationCommand *>(msg);

    if(command != nullptr) {
        switch(command->getCommandType()) {
            case SET_TARGET:
            {
                targetName = command->getTarget();
                sendPacket();
                break;
            }
            case SET_PAYLOAD:
            {
                const FieldsChunk* messagePayload = command->getPayloadTemplate();
                if(messagePayload != nullptr) {
                    if(payloadTemplate != nullptr) {
                        delete payloadTemplate;
                    }
                    payloadTemplate = (FieldsChunk*) messagePayload->dup();;
                }
                sendPacket();
                break;
            }
            case SEND_MESSAGE:
                sendPacket(command->getPayloadTemplate(), command->getTarget());
                break;
        }
        if(command->getPayloadTemplate() != nullptr) {
            delete command->getPayloadTemplate();
        }
        delete msg;
    }
    else {
        UdpBasicApp::handleMessageWhenUp(msg);
    }
}


void UdpCommunicationApp::sendPacket() {
    if(payloadTemplate != nullptr) {
        sendPacket(nullptr, nullptr);
    }
}


void UdpCommunicationApp::sendPacket(const FieldsChunk* payload, const char *target) {
    if(!socket.isOpen()) {
        return;
    }

    if(payload == nullptr) {
        if(payloadTemplate != nullptr) {
            payload = payloadTemplate;
        } else {
            return;
        }
    }
    if(target == nullptr) {
        target = targetName.c_str();
    }

    /*Default package setup*/
    Packet *packet = new Packet("DroneMessage");
    if(dontFragment)
        packet->addTag<FragmentationReq>()->setDontFragment(true);
    packet->setName(this->getParentModule()->getFullName());

    if(payload != nullptr) {
        packet->insertAtBack(payload->dupShared());


        L3Address destAddr;
        if(target != nullptr && strlen(target)  > 0) {
            // Else sends message to the specific target
            L3AddressResolver().tryResolve(target, destAddr);
        } else {
            // No specific target means the message should go to the multicast address
            destAddr = Ipv4Address("224.0.0.9");
        }

        emit(packetSentSignal, packet);
        socket.sendTo(packet, destAddr, destPort);
        numSent++;
    }
}

void UdpCommunicationApp::processPacket(Packet *pk) {
    // Ignore messages not in address list
    if(std::find(destAddressStr.begin(), destAddressStr.end(), std::string(pk->getFullName())) == destAddressStr.end()) {
        delete pk;
        return;
    }

    // Checks if the output side of the "inout" protocolGate is connected
    if(gate("protocolGate$o")->isConnected()) {
        send(pk->dup(), gate("protocolGate$o"));
    }

    emit(packetReceivedSignal, pk);
    delete pk;
}

} // namespace inet

