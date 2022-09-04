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
#include "../../applications/mamapp/BMeshPacket_m.h"
#include "../messages/internal/CommunicationCommand_m.h"
#include "../messages/internal/MobilityCommand_m.h"
#include "CentralizedQProtocol.h"

#include <numeric>

namespace projeto {

Define_Module(CentralizedQProtocol);

void CentralizedQProtocol::initialize(int stage)
{
    CommunicationProtocolBase::initialize(stage);

    if(stage == INITSTAGE_LOCAL) {
        // Signal that carries current data load and is emitted every time it is updated
        dataLoadSignalID = registerSignal("dataLoad");
        emit(dataLoadSignalID, 0);

        learning = dynamic_cast<CentralizedQLearning*>(getModuleByPath("learner"));
        agentId = learning->registerAgent(this);

        WATCH(agentId);
        WATCH(currentState.first);
        WATCH_VECTOR(currentState.second);

        WATCH(currentControl.first);
        WATCH(currentControl.second);
    }
    if(stage == 1) {
        std::vector<unsigned int> emptyVector = {};
        for(int i=0;i<learning->agentCount() + learning->sensorCount();i++) {
            emptyVector.push_back(0);
        }
        currentState.second = emptyVector;
    }
}

void CentralizedQProtocol::handleTelemetry(Telemetry *telemetry) {
    // Computing tour - This only happens once when the drones read their missions
    if(telemetry->hasObject("tourCoords")) {
        tour = *(std::vector<Coord>*) telemetry->par("tourCoords").pointerValue();

        // Pre-computing what fraction of the total tour each coord represents
        const Coord *lastCoord = &tour[0];
        double currentCoordDistance = 0;

        // Starting by computing the cumulative distance at each coord
        for(const Coord& coord : tour) {
            currentCoordDistance += lastCoord->distance(coord);
            tourPercentages.push_back(currentCoordDistance);
            lastCoord = &coord;
        }

        // Finishing by dividing that cumulative distance by the total distance
        int index = 0;
        for(double distance : tourPercentages) {
            tourPercentages[index++] = distance / currentCoordDistance;
        }

    }

    // If the UAV hasn't started his path yet, his movement state is 0
    if(telemetry->getLastWaypointID() == -1) {
        currentState.first = 0;
        return;
    }

    // Updating movement component of the global state
    Coord currentWaypoint = tour[telemetry->getLastWaypointID()];
    Coord nextWaypoint = tour[telemetry->getNextWaypointID()];

    double relativeDistance = currentWaypoint.distance(Coord(telemetry->getCurrentX(), telemetry->getCurrentY(), telemetry->getCurrentZ()));
    relativeDistance /= currentWaypoint.distance(nextWaypoint);
    relativeDistance += tourPercentages[telemetry->getLastWaypointID()] * (telemetry->isReversed() ? -1 : 1);
    currentState.first = relativeDistance;

    lastTelemetry = *telemetry;
}

void CentralizedQProtocol::applyCommand(const LocalControl& control) {
    Enter_Method_Silent();
    // Ignores commands if the UAV hasn't started the tour yet. The last waypoint is -1 when the UAV
    // hasn't reached the first waypoint in the mission yet
    if(lastTelemetry.getLastWaypointID() == -1) {
        return;
    }

    // Sets the completed flag to false to represent the command just received
    hasCompletedControl = false;

    // If the communication component of the control is not zero, tries to communicate with that
    // agent
    if(control.second != 0) {
        communicate(control.second, UAV, SHARE);
    }

    // Checks if the mobility component of the control commands the agent to travel in a ridection it is not
    // already traveling in. In that case, the agent reverses
    if(control.first == 0 && lastTelemetry.isReversed()) {
        reverse();
    } else if(control.first == 1 && !lastTelemetry.isReversed()) {
        reverse();
    }

    // Saves the received control
    currentControl = control;
}

void CentralizedQProtocol::handlePacket(Packet *pk) {
    auto payload = dynamicPtrCast<const CentralizedQMessage>(pk->peekAtBack());

    // Ignores packages that are not destined for this agent's id (or id -1 which sends messages to every node) or are not destined
    // to this node's type
    if(payload != nullptr && (payload->getTargetId() == agentId || payload->getTargetId() == -1) && payload->getTargetNodeType() == UAV) {
        switch(payload->getMessageType()) {
        // SHARE packets are handled by adding their contents to the agent's state and sending an ACK message in reply
        case SHARE:
        {
            // Adding packages to storage after they are received
            int index = payload->getNodeId() + (payload->getNodeType() == SENSOR ? 0 : learning->sensorCount());
            currentState.second[index] += payload->getPacketLoad();

            long sum = 0;
            for(unsigned int value : currentState.second) {
                sum += value;
            }

            communicate(payload->getNodeId(), payload->getNodeType(), ACK);
            emit(dataLoadSignalID, sum);
            break;
        }
        // RECEIVE messages are handled by responding to the message with a SHARE message containing the agent's
        // held data
        case RECEIVE:
        {
            communicate(payload->getNodeId(), payload->getNodeType(), SHARE);
            break;
        }
        // ACK messages are handled by removing the packets contained in the agent as this message is sent
        // in response to a SHARE message.
        case ACK:
        {
            // Zeroing all package content when an ACK is received
            for(int i=0;i<currentState.second.size();i++) {
                currentState.second[i] = 0;
            }
            emit(dataLoadSignalID, 0);
            if (payload->getNodeType() == UAV) {
                hasCompletedControl = true;
            }
            break;
        }
        }
    }
}

void CentralizedQProtocol::communicate(int targetAgent, NodeType targetType, MessageType messageType) {
    // Agent refuses to talk to itself
    if(targetAgent == agentId && targetType == UAV) {
        return;
    }

    CentralizedQMessage *payload = new CentralizedQMessage();
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());


    payload->setNodeType(UAV);
    payload->setNodeId(agentId);
    payload->setMessageType(messageType);
    payload->setTargetNodeType(targetType);
    payload->setTargetId(targetAgent);

    // If the message type is SHARE, include the agent's packet load in the message.
    // Not including this on other messages saves computation.
    if(messageType == MessageType::SHARE) {
        double packetLoad = 0;
        for(int value : currentState.second) {
            packetLoad += value;
        }
        payload->setPacketLoad(packetLoad);
    }


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
} //namespace
