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

#include "UdpMobileNodeCommunicationApp.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "inet/applications/base/ApplicationPacket_m.h"

#include "MobileNode.h"
#include "base/UdpBasicAppMobileNode.h"
#include "../../protocols/messages/internal/CommunicationCommand_m.h"

using namespace inet;

namespace projeto {

Define_Module(UdpMobileNodeCommunicationApp);

void UdpMobileNodeCommunicationApp::initialize(int stage) {
    UdpBasicAppMobileNode::initialize(stage);
}

void UdpMobileNodeCommunicationApp::setSocketOptions() {
    UdpBasicAppMobileNode::setSocketOptions();
    // Joining multicast group used to communicate multicast messages to other drones
    socket.joinMulticastGroup(Ipv4Address("224.0.0.9"));
}

void UdpMobileNodeCommunicationApp::handleMessageWhenUp(cMessage *msg) {
    CommunicationCommand *command = dynamic_cast<CommunicationCommand *>(msg);

    if(command != nullptr) {
        switch(command->getCommandType()) {
            case SET_TARGET:
            {
                targetName = strdup(command->getTarget());
                sendPacket();
                break;
            }
            case SET_PAYLOAD:
            {
                if(payloadTemplate != nullptr) {
                    delete payloadTemplate;
                }
                const FieldsChunk *messagePayload = command->getPayloadTemplate();
                if(messagePayload != nullptr) {
                    payloadTemplate = (FieldsChunk*) messagePayload->dup();
                }
                delete messagePayload;
                sendPacket();
                break;
            }
            case SEND_MESSAGE:
                sendPacket(command->getPayloadTemplate(), strdup(command->getTarget()));
                break;
        }

        cancelAndDelete(msg);
    }
    else {
        UdpBasicAppMobileNode::handleMessageWhenUp(msg);
    }
}


void UdpMobileNodeCommunicationApp::sendPacket(const FieldsChunk* payload, char *target) {
    if(!socket.isOpen()) {
        return;
    }

    if(payload == nullptr) {
        payload = payloadTemplate;
    }
    if(target == nullptr) {
        target = targetName;
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

void UdpMobileNodeCommunicationApp::processPacket(Packet *pk) {
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

