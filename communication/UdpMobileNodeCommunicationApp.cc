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
#include "MobileNode.h"
#include "Order_m.h"
#include "inet/applications/base/ApplicationPacket_m.h"
#include "UdpBasicAppMobileNode.h"

using namespace inet;

namespace projeto {

Define_Module(UdpMobileNodeCommunicationApp);

void UdpMobileNodeCommunicationApp::initialize(int stage) {
    UdpBasicAppMobileNode::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        timeoutDuration = par("timeoutDuration");
        dataCapacity = par("dataCapacity");

        // Signal that carries current data load and is emitted every time it is updated
        dataLoadSignalID = registerSignal("dataLoad");
        emit(dataLoadSignalID, currentDataLoad);
    }
}

void UdpMobileNodeCommunicationApp::setSocketOptions() {
    UdpBasicAppMobileNode::setSocketOptions();
    // Joining multicast group used to communicate multicast messages to other drones
    socket.joinMulticastGroup(Ipv4Address("224.0.0.9"));
}

void UdpMobileNodeCommunicationApp::handleMessageWhenUp(cMessage *msg) {
    Telemetry *telemetry = dynamic_cast<Telemetry *>(msg);

    if(telemetry != nullptr) {
        currentTelemetry = *telemetry;

        // Drops data load after the drone arrives at the first waypoint
        if(currentTelemetry.getLastWaypointID() == 0) {
            currentDataLoad = 0;
            emit(dataLoadSignalID, currentDataLoad);
        }

        // Telemetry during timeout is not stable, it may contain information that
        // has changed after message exchange started
        if(checkAndUpdateTimeout()) {
            lastStableTelemetry = *telemetry;
        }

        cancelAndDelete(msg);
    }
    else {
        UdpBasicAppMobileNode::handleMessageWhenUp(msg);
    }
}


void UdpMobileNodeCommunicationApp::sendPacket() {
    /*Default package setup*/
    Packet *packet = new Packet("DroneMessage");
    if(dontFragment)
        packet->addTag<FragmentationReq>()->setDontFragment(true);
    packet->setName(this->getParentModule()->getFullName());
    const auto& payload = makeShared<MobileNodeMessage>();
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());

    // Sets the reverse flag on the payload
    payload->setReversed(lastStableTelemetry.isReversed());
    payload->setNextWaypointID(lastStableTelemetry.getNextWaypointID());
    payload->setLastWaypointID(lastStableTelemetry.getLastWaypointID());


    if(checkAndUpdateTimeout()) {
        UdpMobileNodeCommunicationApp::sendHeartbeat(payload);
    }
    else {
        if(isRequested) {
            UdpMobileNodeCommunicationApp::sendPairRequest(payload, tentativeTarget);
        } else {
            if(tentativeTarget != -1) {
               UdpMobileNodeCommunicationApp::sendPairConfirm(payload, tentativeTarget);
            } else {
                return;
            }
        }
    }

    packet->insertAtBack(payload);
    L3Address destAddr;
    if(tentativeTarget == -1) {
        // No specific target means the message should go to the multicast address
        destAddr = Ipv4Address("224.0.0.9");
    } else {
        // Else sends message to the specific target
        L3AddressResolver().tryResolve(tentativeTargetName.c_str(), destAddr);
    }

    emit(packetSentSignal, packet);
    socket.sendTo(packet, destAddr, destPort);
    numSent++;
}

void UdpMobileNodeCommunicationApp::sendHeartbeat(inet::IntrusivePtr<inet::MobileNodeMessage> payload) {
    payload->setMessageType(MessageType::HEARTBEAT);
    payload->setSourceID(this->getParentModule()->getIndex());
    std::cout << payload->getSourceID() << " sending heartbeat" << endl;
}
void UdpMobileNodeCommunicationApp::sendPairRequest(inet::IntrusivePtr<inet::MobileNodeMessage> payload, int target) {
    payload->setMessageType(MessageType::PAIR_REQUEST);
    payload->setSourceID(this->getParentModule()->getIndex());
    payload->setDestinationID(target);
    std::cout << payload->getSourceID() << " sending pair request to " << payload->getDestinationID() << endl;
}
void UdpMobileNodeCommunicationApp::sendPairConfirm(inet::IntrusivePtr<inet::MobileNodeMessage> payload, int target) {
    payload->setMessageType(MessageType::PAIR_CONFIRM);
    payload->setSourceID(this->getParentModule()->getIndex());
    payload->setDestinationID(target);
    payload->setDataLength(std::min(currentDataLoad, 10));

    std::cout << payload->getSourceID() << " sending pair confirmation to " << payload->getDestinationID() << endl;
}

void UdpMobileNodeCommunicationApp::processPacket(Packet *pk) {
    auto payload = pk->peekAtBack<MobileNodeMessage>(B(14), 1);

    // Ignore messages not in address list
    if(std::find(destAddressStr.begin(), destAddressStr.end(), std::string(pk->getFullName())) == destAddressStr.end()) {
        delete pk;
        return;
    }

    if(payload != nullptr) {
        switch(payload->getMessageType()) {
            case MessageType::HEARTBEAT:
            {
                if(!checkAndUpdateTimeout() && lastTarget != payload->getSourceID()) {
                    isTimedout = false;
                }

                if(checkAndUpdateTimeout()) {
                    // Only accepts requests if you are going to the same waypoint as drone or you are going to the waypoint it came from
                    if(lastStableTelemetry.getNextWaypointID() == payload->getNextWaypointID() ||
                            lastStableTelemetry.getNextWaypointID() == payload->getLastWaypointID()) {
                        tentativeTarget = payload->getSourceID();
                        tentativeTargetName = pk->getName();
                        isTimedout = true;
                        timeoutStart = simTime();
                        isRequested = true;

                        std::cout << this->getParentModule()->getIndex() << " recieved heartbeat from " << tentativeTarget << endl;

                        sendPacket();
                    }
                }
                break;
            }
            case MessageType::PAIR_REQUEST:
            {
                if(payload->getDestinationID() != this->getParentModule()->getIndex()) {
                    break;
                }

                if(isTimedout) {
                    if(payload->getSourceID() == tentativeTarget) {
                        std::cout << payload->getDestinationID() << " recieved a pair request while timed out from " << payload->getSourceID() << endl;
                        isRequested = false;
                        sendPacket();
                    }
                } else {
                    std::cout << payload->getDestinationID() << " recieved a pair request while not timed out from  " << payload->getSourceID() << endl;
                    tentativeTarget = payload->getSourceID();
                    tentativeTargetName = pk->getName();
                    isTimedout = true;
                    timeoutStart = simTime();
                    isRequested = false;

                    sendPacket();
                }
                break;
            }
            case MessageType::PAIR_CONFIRM:
            {
                if(payload->getSourceID() == tentativeTarget &&
                   payload->getDestinationID() == this->getParentModule()->getIndex()) {
                            std::cout << payload->getDestinationID() << " recieved a pair confirmation from  " << payload->getSourceID() << endl;
                            isRequested = false;
                            if(!isDone) {
                                // If both drones are travelling in the same direction, only the one with the biggest ID inverts
                                if(lastStableTelemetry.isReversed() != payload->getReversed() || this->getParentModule()->getIndex() > payload->getSourceID()) {
                                    sendReverseOrder();

                                    // Exchanging imaginary data to the drone closest to the start of the mission
                                    if(lastStableTelemetry.getLastWaypointID() < payload->getLastWaypointID()) {
                                        // Drone closest to the start gets the data
                                        currentDataLoad = std::min(dataCapacity, currentDataLoad + payload->getDataLength());
                                    } else {
                                        // Drone farthest away loses the data
                                        currentDataLoad = std::max(0, currentDataLoad - payload->getDataLength());
                                    }

                                    // Updating data load
                                    emit(dataLoadSignalID, currentDataLoad);
                                }


                            }
                            isDone = true;
               }
                break;
            }
            case MessageType::BEARER:
            {
                std::cout << this->getParentModule()->getIndex() << " recieved bearer request from  " << pk->getName() << endl;
                currentDataLoad = std::min(dataCapacity, currentDataLoad + payload->getDataLength());
                emit(dataLoadSignalID, currentDataLoad);
                break;
            }
        }
    }
    emit(packetReceivedSignal, pk);
    delete pk;
}

bool UdpMobileNodeCommunicationApp::checkAndUpdateTimeout() {
    if(isTimedout) {
        if(simTime() - timeoutStart < timeoutDuration) {
            return false;
        } else {
            isTimedout = false;
            resetParameters();
            return true;
        }
    } else {
        return true;
    }
}

void UdpMobileNodeCommunicationApp::resetParameters() {
    lastTarget = tentativeTarget;
    tentativeTarget = -1;
    tentativeTargetName = "";
    isRequested = false;
    isDone = false;

    lastStableTelemetry = currentTelemetry;
}


// Sends a reverse order to the mobility module
void UdpMobileNodeCommunicationApp::sendReverseOrder() {
    Order *message = new Order("Reverse Order", 0);
    send(message, gate("mobilityGate$o"));
}

} // namespace inet

