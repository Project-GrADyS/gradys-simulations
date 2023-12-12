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

#include "ZigzagProtocol.h"
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

Define_Module(ZigzagProtocol);

void ZigzagProtocol::initialize(int stage)
{
    CommunicationProtocolBase::initialize(stage);

    if(stage == INITSTAGE_LOCAL) {
        timeoutDuration = par("timeoutDuration");
        // Signal that carries current data load and is emitted every time it is updated
        dataLoadSignalID = registerSignal("dataLoad");
        emit(dataLoadSignalID, currentDataLoad);

        updatePayload();
    }
}

void ZigzagProtocol::handleTelemetry(gradys_simulations::Telemetry *telemetry) {
    currentTelemetry = *telemetry;

    // Telemetry during timeout is not stable, it may contain information that
    // has changed after message exchange started
    if(!isTimedout()) {
        lastStableTelemetry = *telemetry;
    }
    updatePayload();
}

void ZigzagProtocol::handlePacket(Packet *pk) {
    auto payload = pk->peekAtBack<ZigzagMessage>(B(14), 1);

    if(payload != nullptr) {
        bool destinationIsGroundstation = payload->getNextWaypointID() == -1;
        // No communication from other drones matters while the drone is executing
        // or if the drone is recharging/shutdown
        if(currentTelemetry.getCurrentCommand() != -1 && !destinationIsGroundstation) {
            return;
        }
        switch(payload->getMessageType()) {
            case ZigzagMessageType::HEARTBEAT:
            {
                if(isTimedout() && lastTarget != payload->getSourceID() && tentativeTarget != payload->getSourceID()) {
                    resetParameters();
                }

                // If the drone is collecting data, prefer to pair with other drone
                if(communicationStatus == COLLECTING) {
                    resetParameters();
                }

                if(!isTimedout()) {
                    // Only accepts requests if you are going to the same waypoint as drone or you are going to the waypoint it came from
                    // or if the drone is stationary
                    if(lastStableTelemetry.getNextWaypointID() == payload->getNextWaypointID() ||
                            lastStableTelemetry.getNextWaypointID() == payload->getLastWaypointID() ||
                            lastStableTelemetry.getNextWaypointID() == -1 ||
                            payload->getNextWaypointID() == -1) {
                        tentativeTarget = payload->getSourceID();
                        tentativeTargetName = pk->getName();
                        setTarget(tentativeTargetName.c_str());
                        initiateTimeout(timeoutDuration);
                        communicationStatus = REQUESTING;

                        std::cout << this->getParentModule()->getId() << " recieved heartbeat from " << tentativeTarget << endl;
                    }
                }
                break;
            }
            case ZigzagMessageType::PAIR_REQUEST:
            {
                if(payload->getDestinationID() != this->getParentModule()->getId()) {
                    break;
                }

                // If the drone is collecting data, prefer to pair with other drone
                if(communicationStatus == COLLECTING) {
                    resetParameters();
                }

                if(isTimedout()) {
                    if(payload->getSourceID() == tentativeTarget) {
                        std::cout << payload->getDestinationID() << " received a pair request while timed out from " << payload->getSourceID() << endl;
                        communicationStatus = PAIRED;
                    }
                } else {
                    std::cout << payload->getDestinationID() << " received a pair request while not timed out from  " << payload->getSourceID() << endl;
                    tentativeTarget = payload->getSourceID();
                    tentativeTargetName = pk->getName();
                    initiateTimeout(timeoutDuration);

                    communicationStatus = PAIRED;
                }
                break;
            }
            case ZigzagMessageType::PAIR_CONFIRM:
            {
                if(payload->getSourceID() == tentativeTarget &&
                   payload->getDestinationID() == this->getParentModule()->getId()) {
                    std::cout << payload->getDestinationID() << " recieved a pair confirmation from  " << payload->getSourceID() << endl;
                    if(communicationStatus != PAIRED_FINISHED) {
                        // If both drones are travelling in the same direction, only the one with the biggest ID inverts
                        if(lastStableTelemetry.isReversed() != payload->getReversed() || this->getParentModule()->getId() > payload->getSourceID()) {
                            sendReverseOrder();

                            // Exchanging imaginary data to the drone closest to the start of the mission
                            if(lastStableTelemetry.getLastWaypointID() < payload->getLastWaypointID()) {
                                // Drone closest to the start gets the data
                                currentDataLoad = currentDataLoad + payload->getDataLength();
                            } else {
                                // Drone farthest away loses data
                                currentDataLoad = 0;
                            }
                            // Updating data load
                            emit(dataLoadSignalID, currentDataLoad);
                        }
                    }
                    communicationStatus = PAIRED_FINISHED;
               }
                break;
            }
            case ZigzagMessageType::BEARER:
            {
                if(!isTimedout() && communicationStatus == FREE) {
                    std::cout << this->getParentModule()->getId() << " recieved bearer request from  " << pk->getName() << endl;
                    currentDataLoad = currentDataLoad + payload->getDataLength();
                    stableDataLoad = currentDataLoad;
                    emit(dataLoadSignalID, currentDataLoad);
                    initiateTimeout(timeoutDuration);
                    communicationStatus = COLLECTING;
                }
                break;
            }
        }
        updatePayload();
    }
}

void ZigzagProtocol::sendReverseOrder() {
    MobilityCommand *command = new MobilityCommand();
    command->setCommandType(REVERSE);

    sendCommand(command);
}

void ZigzagProtocol::updatePayload() {
    ZigzagMessage *payload = new ZigzagMessage();
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());

    // Sets the reverse flag on the payload
    payload->setReversed(lastStableTelemetry.isReversed());
    payload->setNextWaypointID(lastStableTelemetry.getNextWaypointID());
    payload->setLastWaypointID(lastStableTelemetry.getLastWaypointID());
    payload->setSourceID(this->getParentModule()->getId());

    if(!isTimedout() && communicationStatus != FREE) {
        communicationStatus = FREE;
    }

    switch(communicationStatus) {
        case FREE:
        {
            payload->setMessageType(ZigzagMessageType::HEARTBEAT);
            std::cout << payload->getSourceID() << " set to heartbeat" << endl;
            break;
        }
        case REQUESTING:
        {
            payload->setMessageType(ZigzagMessageType::PAIR_REQUEST);
            payload->setDestinationID(tentativeTarget);
            std::cout << payload->getSourceID() << " set to pair request to " << payload->getDestinationID() << endl;
            break;
        }
        case PAIRED:
        case PAIRED_FINISHED:
        {
            payload->setMessageType(ZigzagMessageType::PAIR_CONFIRM);
            payload->setDestinationID(tentativeTarget);
            payload->setDataLength(stableDataLoad);

            std::cout << payload->getSourceID() << " set to pair confirmation to " << payload->getDestinationID() << endl;
            break;
        }
        case COLLECTING:
            break;
    }

    // Only send the update command if the payload has actually changed
    if(payload->getMessageType() != lastPayload.getMessageType() ||
            payload->getSourceID() != lastPayload.getSourceID() ||
            payload->getDestinationID() != lastPayload.getDestinationID() ||
            payload->getNextWaypointID() != lastPayload.getNextWaypointID() ||
            payload->getLastWaypointID() != lastPayload.getLastWaypointID() ||
            payload->getReversed() != lastPayload.getReversed()) {
        lastPayload = *payload;

        CommunicationCommand *command = new CommunicationCommand();
        command->setCommandType(SET_PAYLOAD);
        command->setPayloadTemplate(payload);
        sendCommand(command);
    } else {
        delete payload;
    }

}

void ZigzagProtocol::setTarget(const char *target) {
    CommunicationCommand *command = new CommunicationCommand();
    command->setCommandType(SET_TARGET);
    command->setTarget(target);
    sendCommand(command);
}

bool ZigzagProtocol::isTimedout() {
    bool oldValue = timeoutSet;
    bool value = CommunicationProtocolBase::isTimedout();
    if(!value && oldValue) {
        resetParameters();
    }
    return value;
}

void ZigzagProtocol::resetParameters() {
    timeoutSet = false;
    lastTarget = tentativeTarget;
    tentativeTarget = -1;
    tentativeTargetName = "";
    setTarget("");
    communicationStatus = FREE;

    lastStableTelemetry = currentTelemetry;
    stableDataLoad = currentDataLoad;

    updatePayload();
}
} //namespace
