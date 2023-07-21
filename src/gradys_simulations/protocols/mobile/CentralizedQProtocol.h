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

#ifndef __gradys_simulations_CentralizedQProtocol_H_
#define __gradys_simulations_CentralizedQProtocol_H_

#include <omnetpp.h>
#include "gradys_simulations/protocols/base/CommunicationProtocolBase.h"
#include "gradys_simulations/protocols/auxiliary/CentralizedQLearning.h"
#include "gradys_simulations/protocols/messages/internal/Telemetry_m.h"
#include "gradys_simulations/protocols/messages/network/CentralizedQMessage_m.h"
#include "inet/common/geometry/common/Coord.h"

using namespace omnetpp;

namespace gradys_simulations {

/*
 * CentralizedQProtocol implements a protocol that recieves and sends DadcaMessages to simulate a
 * drone collecting data from sensors and sharing it with other drones. This protocol implements
 * the DADCA protocol.
 */
class CentralizedQProtocol : public CommunicationProtocolBase, public CentralizedQLearning::CentralizedQAgent
{
public:
    virtual double getCurrentPosition () override { return currentDistance; };

    virtual double getMaximumPosition () override { return totalMissionLength; };

    uint32_t getCollectedPackets() override { return collectedPackets; };

    uint32_t getMaxCollectedPackets() override { return packetLimit; };

    // Applies a command to the agent
    virtual void applyCommand(const LocalControl& command) override;

    // Informs the centralized Q learning module if the module is ready to receive a new
    // set of commands
    bool isReady() override { return hasCompletedMobility && hasStartedMission; }

protected:
    // Reference to the learning module
    CentralizedQLearning* learning;

    // ID of this agent
    int agentId;

    // Variable used to inform the learning module if the last
    // control received was completed
    int commandTargetDistance = 0;
    bool hasCompletedMobility = true;
    bool hasStartedMission = false;

    // Saving the current state and control
    LocalState currentState = {};
    double currentDistance = 0;
    uint32_t collectedPackets = 0;

    // Maximum number of packets
    int packetLimit;

    LocalControl currentControl = {};

    // Saving the last telemetry received
    Telemetry* lastTelemetry = nullptr;

    // Information about the waypoint mission the UAVs are following. The first variable is a list
    // of waypoint coordinates and the second variable is a list of numbers representing the fraction
    // of the total tour that the coordinate in that index represents. This is used to speed up the
    // computation of the mobility component of the current state
    std::vector<Coord> tour;
    std::vector<double> tourDistances;
    double totalMissionLength;
    // This is the distance interval received from the centralized learning module. It is used to
    // calculate a discrete mobility state based on the continuous position of the agent
    double distanceInterval;

    double communicationStorageInterval;


    // Variables that control the agent's request timer. Each time this timer fires the agent shares
    // to the whole network a message of REQUEST type.
    simtime_t requestInterval;
    cMessage* requestTimer = new cMessage();

    simtime_t communicationDelay;
    int requestTargetId = -1;
    double requestTargetPosition = 0;
    cMessage* communicationDelayTimer = new cMessage();

protected:
    // OMNeT++ and INET functions
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return 3; };
    virtual void handleMessage(cMessage *msg) override;

    // Handles telemetry received by the mobility module and uses it to compute the mobility component
    // of the current state. The most current telemetry message is saved.
    virtual void handleTelemetry(Telemetry *telemetry) override;

    // Handles packets received by the communication module. These packet can be from other UAVs, sensors
    // and the ground station. This function implements the specific behavior triggered by the reception
    // of these packet types.
    virtual void handlePacket(Packet *pk) override;

    // Helper function that sends a network messages with content specified by the function's parameters
    virtual void communicate(int targetAgent, NodeType targetType, MessageType messageType);

    // Helper function that reverses the course of the UAVs movement
    virtual void reverse();
    virtual void stop();
    virtual void resume();
public:
    simsignal_t dataLoadSignalID;
};

} //namespace

#endif
