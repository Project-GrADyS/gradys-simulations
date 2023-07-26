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
#include "gradys_simulations/protocols/messages/internal/CommunicationCommand_m.h"
#include "gradys_simulations/protocols/messages/internal/MobilityCommand_m.h"
#include "CentralizedQProtocol.h"

#include <algorithm>
#include <numeric>

namespace gradys_simulations {

Define_Module(CentralizedQProtocol);

void CentralizedQProtocol::initialize(int stage)
{
    CommunicationProtocolBase::initialize(stage);

    if(stage == INITSTAGE_LOCAL) {
        // Signal that carries current data load and is emitted every time it is updated
        dataLoadSignalID = registerSignal("dataLoad");
        emit(dataLoadSignalID, 0);

        learning = dynamic_cast<CentralizedQLearning*>(getModuleByPath("learner"));

        communicationDelay = par("communicationDelay");

        requestInterval = par("requestInterval");
        // Introduces a bit for randomness to the first (and subsequent) messages, this helps prevent
        // collision when sending messages
        scheduleAt(uniform(0,communicationDelay), requestTimer);
        packetLimit = par("packetLimit");

        WATCH(agentId);
        WATCH(currentState.mobility);
        WATCH(currentState.communication);

        WATCH(currentDistance);
        WATCH(collectedPackets);

        WATCH(commandTargetDistance);
        WATCH(hasCompletedMobility);

        WATCH(currentControl.mobility);
    }
    if(stage == 1) {
        auto info = learning->registerAgent(this);
        agentId = info.agentId;
        distanceInterval = info.distanceInterval;
        communicationStorageInterval = info.communicationStorageInterval;
    }
    if(stage == 2) {
        collectedPackets = 0;
        currentState.communication = 0;
    }
}

void CentralizedQProtocol::handleMessage(cMessage *msg) {
    if(msg->isSelfMessage()) {
        if(msg == requestTimer) {
            communicate(-1, ALL, REQUEST);
            scheduleAt(simTime() + requestInterval, requestTimer);
            return;
        } else if (msg == communicationDelayTimer) {
            communicate(requestTargetId, AGENT, SHARE);
            requestTargetId = -1;
            // Zeroing all package content when an ACK is received
            collectedPackets = 0;
            emit(dataLoadSignalID, 0);
            return;
        }
    }
    CommunicationProtocolBase::handleMessage(msg);
}

void CentralizedQProtocol::handleTelemetry(Telemetry *telemetry) {
    // Computing tour - This only happens once when the drones read their missions
    if(telemetry->hasObject("tourCoords")) {
        auto tourPointer = (std::vector<Coord>*)(telemetry->par("tourCoords").pointerValue());
        tour = *tourPointer;

        delete tourPointer;

        // Pre-computing what fraction of the total tour each coord represents
        const Coord *lastCoord = &tour[0];
        double currentCoordDistance = 0;

        // Starting by computing the cumulative distance at each coord
        for(const Coord& coord : tour) {
            currentCoordDistance += lastCoord->distance(coord);
            tourDistances.push_back(currentCoordDistance);
            lastCoord = &coord;
        }
        totalMissionLength = tourDistances[tour.size() - 1];
    }

    // If the UAV hasn't started his path yet, his movement state is 0
    if(telemetry->getLastWaypointID() == -1) {
        currentDistance = 0;
        hasStartedMission = false;
        return;
    } else {
        hasStartedMission = true;
    }

    // Updating movement component of the global state
    Coord currentWaypoint = tour[telemetry->getLastWaypointID()];
    Coord nextWaypoint = tour[telemetry->getNextWaypointID()];

    int fartherWaypointIndex = std::max(telemetry->getLastWaypointID(), telemetry->getNextWaypointID());
    int closerWaypointIndex = std::min(telemetry->getLastWaypointID(), telemetry->getNextWaypointID());

    double distance = tourDistances[closerWaypointIndex];

    // If the agent is between two waypoints, add that to the total distance
    if(fartherWaypointIndex != closerWaypointIndex) {
        distance += tour[closerWaypointIndex].distance(Coord(telemetry->getCurrentX(), telemetry->getCurrentY(), telemetry->getCurrentZ()));
    }
    currentDistance = distance;

    uint16_t targetDistance = commandTargetDistance * distanceInterval;
    if (std::abs(currentDistance - targetDistance) <= 10 && !hasCompletedMobility) {
        stop();
        hasCompletedMobility = true;
    }
    if (lastTelemetry != nullptr) {
        delete lastTelemetry;
    }
    lastTelemetry = telemetry->dup();
}

void CentralizedQProtocol::applyCommand(const LocalControl& control) {
    Enter_Method_Silent();


    // Ignores commands if the UAV hasn't started the tour yet. The last waypoint is -1 when the UAV
    // hasn't reached the first waypoint in the mission yet
    if(lastTelemetry && lastTelemetry->getLastWaypointID() == -1) {
        return;
    }


    // Sets the completed flags to false when receiving a new command
    hasCompletedMobility = false;
    resume();

    // Checks if the mobility component of the control commands the agent to travel in a ridection it is not
    // already traveling in. In that case, the agent reverses
    if(control.mobility == 0 && (lastTelemetry && lastTelemetry->isReversed())) {
        reverse();
    } else if(control.mobility == 1 && !(lastTelemetry && lastTelemetry->isReversed())) {
        reverse();
    }

    // We add a bit of extra distance to our current distance to account for overshooting the target when reversed
    commandTargetDistance = std::floor((currentDistance + (0.5 * distanceInterval)) / distanceInterval) + (control.mobility == 0 ? 1 : -1);


    commandTargetDistance = std::max<int>(0, commandTargetDistance);
    commandTargetDistance = std::min<int>(std::floor(totalMissionLength / distanceInterval), commandTargetDistance);


    // Saves the received control
    currentControl = control;
}

void CentralizedQProtocol::handlePacket(Packet *pk) {
    auto payload = dynamicPtrCast<const CentralizedQMessage>(pk->peekAtBack(B(15), 1));

    // Ignores packages that are not destined for this agent's id (or id -1 which sends messages to every node) or are not destined
    // to this node's type
    if(payload != nullptr && (payload->getTargetId() == agentId || payload->getTargetId() == -1)
            && (payload->getTargetNodeType() == AGENT || payload->getTargetNodeType() == ALL)) {
        switch(payload->getMessageType()) {
        // SHARE packets are handled by adding their contents to the agent's state and sending an ACK message in reply
        case SHARE:
        {
            // Adding packets to storage
            collectedPackets += payload->getPacketLoad();

            if(collectedPackets > packetLimit) {
                collectedPackets = packetLimit;
            }

            emit(dataLoadSignalID, static_cast<long>(collectedPackets));
            break;
        }
        // REQUEST messages are handled by responding to the message with a SHARE message containing the agent's
        // held data
        case REQUEST:
        {
            // No point in answering requests if you have nothing to send
            if(collectedPackets == 0) {
                break;
            }

            // If it's not an agent, fulfill the request
            if (payload->getNodeType() != AGENT) {
                communicate(payload->getNodeId(), payload->getNodeType(), SHARE);
                // Zeroing all package content when an ACK is received
                collectedPackets = 0;
                emit(dataLoadSignalID, 0);
            }
            // If it is, only fulfill if it is closer to the GS than you
            else if (payload->getNodeType() == AGENT && currentDistance > payload->getNodePosition()) {
                if (payload->getNodePosition() < requestTargetPosition || requestTargetId == -1) {
                    requestTargetId = payload->getNodeId();
                    requestTargetPosition = payload->getNodePosition();
                }
                if(!communicationDelayTimer->isScheduled()) {
                    scheduleAt(simTime() + communicationDelay, communicationDelayTimer);
                }
            }
            break;
        }
        // ACK messages are handled by removing the packets contained in the agent as this message is sent
        // in response to a SHARE message.
        case ACK:
        {
            // Zeroing all package content when an ACK is received
            collectedPackets = 0;
            emit(dataLoadSignalID, 0);
            break;
        }
        }
    }
}

void CentralizedQProtocol::communicate(int targetAgent, NodeType targetType, MessageType messageType) {
    // Agent refuses to talk to itself
    if(targetAgent == agentId && targetType == AGENT) {
        return;
    }

    CentralizedQMessage *payload = new CentralizedQMessage();
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());


    payload->setNodeType(AGENT);
    payload->setNodeId(agentId);
    payload->setMessageType(messageType);
    payload->setTargetNodeType(targetType);
    payload->setTargetId(targetAgent);
    payload->setNodePosition(currentDistance);
    payload->setPacketLoad(collectedPackets);


    CommunicationCommand *command = new CommunicationCommand();
    command->setCommandType(SEND_MESSAGE);
    command->setPayloadTemplate(payload);
    command->setTarget(nullptr);
    sendCommand(command);
}

void CentralizedQProtocol::reverse() {
    MobilityCommand *command = new MobilityCommand();
    command->setCommandType(REVERSE);

    sendCommand(command);
}

void CentralizedQProtocol::stop() {
    MobilityCommand *command = new MobilityCommand();
    command->setCommandType(FORCE_SHUTDOWN);

    sendCommand(command);
}

void CentralizedQProtocol::resume() {
    MobilityCommand *command = new MobilityCommand();
    command->setCommandType(WAKE_UP);

    sendCommand(command);
}
} //namespace
