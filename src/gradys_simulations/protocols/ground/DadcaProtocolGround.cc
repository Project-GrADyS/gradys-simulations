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

#include "inet/common/ModuleAccess.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "inet/applications/base/ApplicationPacket_m.h"
#include "gradys_simulations/applications/mamapp/BMeshPacket_m.h"
#include "DadcaProtocolGround.h"

namespace gradys_simulations {

Define_Module(DadcaProtocolGround);

void DadcaProtocolGround::initialize(int stage)
{
    CommunicationProtocolBase::initialize(stage);

    if(stage == INITSTAGE_LOCAL) {
        timeoutDuration = par("timeoutDuration");
        dataLoggingInterval = par("dataLoggingInterval");
        scheduleAt(simTime() + dataLoggingInterval, dataLoggingTimer);

        int duration = timeoutDuration.inUnit(SimTimeUnit::SIMTIME_S);
        // Signal that carries current data load and is emitted every time it is updated
        dataLoadSignalID = registerSignal("dataLoad");
        throughputSignalID = registerSignal("throughput");
        emit(dataLoadSignalID, currentDataLoad);

        updatePayload();

        WATCH(leftNeighbours);
        WATCH(rightNeighbours);
        WATCH(communicationStatus);
        WATCH(tentativeTarget);
        WATCH(lastTarget);
        WATCH(currentDataLoad);
    }
}

void DadcaProtocolGround::handleMessage(cMessage *msg) {
    if(msg == dataLoggingTimer) {
        emit(throughputSignalID, static_cast<double>(currentDataLoad) / simTime());
        scheduleAt(simTime() + dataLoggingInterval, dataLoggingTimer);
    } else {
        CommunicationProtocolBase::handleMessage(msg);
    }
}

void DadcaProtocolGround::handleTelemetry(gradys_simulations::Telemetry *telemetry) {
    // Starts a timeout right after the drone has completed a command (Rendevouz)
    if(currentTelemetry.getCurrentCommand() != -1 && telemetry->getCurrentCommand() == -1) {
        resetParameters();
        initiateTimeout(timeoutDuration);
    }

    // Records if the drone has reached an edge and erases the neighbours after that edge
    if(currentTelemetry.getDroneActivity() != REACHED_EDGE && telemetry->getDroneActivity() == REACHED_EDGE) {
        if(telemetry->isReversed()) {
            rightNeighbours = 0;
        } else {
            leftNeighbours = 0;
        }
    }

    // Erases neighbours when drone is recharging or shutdown
    if((currentTelemetry.getDroneActivity() != RECHARGING && telemetry->getDroneActivity() == RECHARGING) ||
            (currentTelemetry.getDroneActivity() != SHUTDOWN && telemetry->getDroneActivity() == SHUTDOWN)) {
        leftNeighbours = rightNeighbours = 0;
    }

    currentTelemetry = *telemetry;

    if(telemetry->hasObject("tourCoords")) {
        tour = *(std::vector<Coord>*) telemetry->par("tourCoords").pointerValue();
    }

    // Telemetry during timeout is not stable, it may contain information that
    // has changed after message exchange started
    if(!isTimedout()) {
        lastStableTelemetry = *telemetry;
    }
    updatePayload();
}

void DadcaProtocolGround::handlePacket(Packet *pk) {
    auto payload = dynamicPtrCast<const DadcaMessage>(pk->peekAtBack(B(34), 1));


    if(payload != nullptr) {
        bool destinationIsGroundstation = payload->getNextWaypointID() == -1;

        switch(payload->getMessageType()) {
            case DadcaMessageType::HEARTBEAT:
            {
                // No communication from other drones matters while the drone is executing
                // or if the drone is recharging/shutdown
                if(currentTelemetry.getCurrentCommand() != -1 && !destinationIsGroundstation) {
                    return;
                }

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
                            destinationIsGroundstation) {
                        tentativeTarget = payload->getSourceID();
                        tentativeTargetName = pk->getName();
                        setTarget(tentativeTargetName.c_str());
                        initiateTimeout(timeoutDuration);
                        communicationStatus = REQUESTING;

                        EV_INFO << this->getParentModule()->getId() << " recieved heartbeat from " << tentativeTarget << endl;
                    }
                }
                break;
            }
            case DadcaMessageType::PAIR_REQUEST:
            {
                // No communication form other drones matters while the drone is executing
                if(currentTelemetry.getCurrentCommand() != -1 && !destinationIsGroundstation) {
                    break;
                }

                if(payload->getDestinationID() != this->getParentModule()->getId()) {
                    break;
                }

                // If the drone is collecting data, prefer to pair with other drone
                if(communicationStatus == COLLECTING) {
                    resetParameters();
                }

                if(isTimedout()) {
                    if(payload->getSourceID() == tentativeTarget) {
                        EV_INFO << payload->getDestinationID() << " recieved a pair request while timed out from " << payload->getSourceID() << endl;
                        communicationStatus = PAIRED;
                    }
                } else {
                    EV_INFO << payload->getDestinationID() << " recieved a pair request while not timed out from  " << payload->getSourceID() << endl;
                    tentativeTarget = payload->getSourceID();
                    tentativeTargetName = pk->getName();
                    initiateTimeout(timeoutDuration);

                    communicationStatus = PAIRED;
                }

                break;
            }
            case DadcaMessageType::PAIR_CONFIRM:
            {
                // No communication form other drones matters while the drone is executing
                if(currentTelemetry.getCurrentCommand() != -1 && !destinationIsGroundstation) {
                    break;
                }


                if(payload->getSourceID() == tentativeTarget &&
                   payload->getDestinationID() == this->getParentModule()->getId()) {
                    EV_INFO << payload->getDestinationID() << " recieved a pair confirmation from  " << payload->getSourceID() << endl;
                    if(communicationStatus != PAIRED_FINISHED) {
                        // If both drones are travelling in the same direction, the pairing is canceled
                        // Doesn't apply if one drone is the groundStation
                        if((lastStableTelemetry.isReversed() != payload->getReversed()) || (tour.size() == 0 || destinationIsGroundstation)) {
                            // Exchanging imaginary data to the drone closest to the start of the mission
                            if(lastStableTelemetry.getLastWaypointID() < payload->getLastWaypointID()) {
                                // Drone closest to the start gets the data
                                currentDataLoad = currentDataLoad + payload->getDataLength();


                                // Doesn't update neighbours if the drone has no waypoints
                                // This prevents counting the groundStation as a drone
                                if(!destinationIsGroundstation) {
                                    // Drone closest to the start updates right neighbours
                                    rightNeighbours = payload->getRightNeighbours() + 1;
                                }
                            } else {
                                // Drone farthest away loses data
                                currentDataLoad = 0;

                                // Doesn't update neighbours if the drone has no waypoints
                                // This prevents counting the groundStation as a drone
                                if(!destinationIsGroundstation) {
                                    // Drone farthest away updates left neighbours
                                    leftNeighbours = payload->getLeftNeighbours() + 1;
                                }
                            }

                            // Only completes redevouz if tour has been recieved or the paired drone has no waypoints
                            // This prevents rendevouz with the groundStation
                            if(tour.size() > 0 && !destinationIsGroundstation) {
                                rendevouz();
                            }
                            // Updating data load
                            emit(dataLoadSignalID, currentDataLoad);
                            communicationStatus = PAIRED_FINISHED;

                        }
                    }
               }
                break;
            }
            case DadcaMessageType::BEARER:
            {
                if(!isTimedout() && communicationStatus == FREE) {
                    EV_INFO << this->getParentModule()->getId() << " recieved bearer request from  " << pk->getName() << endl;
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

    auto mamPayload = dynamicPtrCast<const BMeshPacket>(pk->peekAtBack(B(1), 1));
    if(mamPayload != nullptr) {
        if(!isTimedout() && communicationStatus == FREE) {
            currentDataLoad++;
            stableDataLoad = currentDataLoad;
            emit(dataLoadSignalID, currentDataLoad);
        }
    }
}

void DadcaProtocolGround::rendevouz() {
    // Drone is the left or right one in the pair
    bool isLeft = !lastStableTelemetry.isReversed();

    // Calculates rally point
    std::vector<double> cumulativeDistances;
    double totalDistance = 0;
    Coord lastCoord;
    for(int i = 0; i < tour.size(); i++) {
        Coord coord = tour[i];

        if(i > 0) {
            totalDistance += lastCoord.distance(coord);
        }
        cumulativeDistances.push_back(totalDistance);

        lastCoord = coord;
    }
    int totalNeighbours = leftNeighbours + rightNeighbours;

    // Shared border where the paired drones will meet
    double sharedBorder = (double) leftNeighbours / (totalNeighbours + 1);
    // If the drone is not inverted it will start it's trip from the end
    // of it's segment and them invert
    if(isLeft) {
        sharedBorder += 1.0/(totalNeighbours + 1);
    }
    double sharedBorderDistance = totalDistance * sharedBorder;

    // Determines the waypoint index closest to the shared border
    int waypointIndex;
    for(waypointIndex=0; waypointIndex < cumulativeDistances.size() ; waypointIndex++) {
        if(cumulativeDistances[waypointIndex] > sharedBorderDistance) {
            // Waypoint before the acumulated distance is higher than the
            // shared border distance
            waypointIndex--;
            break;
        }
    }

    // Fraction that devides the waypoint before the shared border and the one after it
    // at the shared border
    double localBorderFraction = 1 - (cumulativeDistances[waypointIndex + 1] - sharedBorderDistance) / (cumulativeDistances[waypointIndex + 1] - cumulativeDistances[waypointIndex]);

    Coord coordsBefore = tour[waypointIndex];
    Coord coordsAfter = tour[waypointIndex + 1];
    Coord sharedBorderCoords = ((coordsAfter - coordsBefore) * localBorderFraction) + coordsBefore;

    bool isAhead = false;

    if(!isLeft) {
        if(lastStableTelemetry.getNextWaypointID() > waypointIndex) {
            isAhead = true;
        }
    } else {
        if(lastStableTelemetry.getLastWaypointID() > waypointIndex) {
            isAhead = true;
        }
    }

    // The rendevouz is divided in three steps

    // First the drones navigate to the waypoint closest to the shared border
    // If the drones are already coming from/going to the waypoint closest they don't need this command
    if((!isLeft || lastStableTelemetry.getLastWaypointID() != waypointIndex) &&
            (isLeft || lastStableTelemetry.getNextWaypointID() != waypointIndex)) {
        MobilityCommand *firstCommand = new MobilityCommand();
        firstCommand->setCommandType(GOTO_WAYPOINT);

        // If the drone is ahead of the shared border navigate to the next waypoint after it
        if(isAhead) {
            firstCommand->setParam1(waypointIndex + 1);
        } else {
            firstCommand->setParam1(waypointIndex);
        }

        sendCommand(firstCommand);
    }

    // Them the drones meet at the shared border
    MobilityCommand *secondCommand = new MobilityCommand();
    secondCommand->setCommandType(GOTO_COORDS);
    secondCommand->setParam1(sharedBorderCoords.x);
    secondCommand->setParam2(sharedBorderCoords.y);
    secondCommand->setParam3(sharedBorderCoords.z);

    // After the drones reach the coords they will be oriented
    // going from waypointIndex to waypointIndex + 1
    secondCommand->setParam4(waypointIndex + 1);
    secondCommand->setParam5(waypointIndex);
    sendCommand(secondCommand);


    // Them the drones reverses
    // Only the drone on the left needs to reverse
    // since both drones are oriented unreversed
    if(isLeft) {
        MobilityCommand *thirdCommand = new MobilityCommand();
        thirdCommand->setCommandType(REVERSE);
        sendCommand(thirdCommand);
    }
}

void DadcaProtocolGround::updatePayload() {
    DadcaMessage *payload = new DadcaMessage();
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());

    // Sets the reverse flag on the payload
    payload->setReversed(lastStableTelemetry.isReversed());
    payload->setNextWaypointID(lastStableTelemetry.getNextWaypointID());
    payload->setLastWaypointID(lastStableTelemetry.getLastWaypointID());

    payload->setLeftNeighbours(leftNeighbours);
    payload->setRightNeighbours(rightNeighbours);

    payload->setSourceID(this->getParentModule()->getId());

    if(!isTimedout() && communicationStatus != FREE) {
        communicationStatus = FREE;
    }

    switch(communicationStatus) {
        case FREE:
        {
            payload->setMessageType(DadcaMessageType::HEARTBEAT);
            EV_INFO << payload->getSourceID() << " set to heartbeat" << endl;
            break;
        }
        case REQUESTING:
        {
            payload->setMessageType(DadcaMessageType::PAIR_REQUEST);
            payload->setDestinationID(tentativeTarget);
            EV_INFO << payload->getSourceID() << " set to pair request to " << payload->getDestinationID() << endl;
            break;
        }
        case PAIRED:
        case PAIRED_FINISHED:
        {
            payload->setMessageType(DadcaMessageType::PAIR_CONFIRM);
            payload->setDestinationID(tentativeTarget);
            payload->setDataLength(stableDataLoad);

            EV_INFO << payload->getSourceID() << " set to pair confirmation to " << payload->getDestinationID() << endl;
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

void DadcaProtocolGround::setTarget(const char *target) {
    CommunicationCommand *command = new CommunicationCommand();
    command->setCommandType(SET_TARGET);
    command->setTarget(target);
    sendCommand(command);
}

bool DadcaProtocolGround::isTimedout() {
    // Blocks the timeout if the drone is currently executing a command
    if(currentTelemetry.getCurrentCommand() != -1) {
        return true;
    }

    bool oldValue = timeoutSet;
    bool value = CommunicationProtocolBase::isTimedout();
    if(!value && oldValue) {
        resetParameters();
    }
    return value;
}

void DadcaProtocolGround::resetParameters() {
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
