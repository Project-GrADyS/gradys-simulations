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

#ifndef __PROJETO_CentralizedQProtocol_H_
#define __PROJETO_CentralizedQProtocol_H_

#include <omnetpp.h>
#include "../base/CommunicationProtocolBase.h"
#include "../auxiliary/CentralizedQLearning.h"
#include "../messages/internal/Telemetry_m.h"
#include "inet/common/geometry/common/Coord.h"

using namespace omnetpp;

namespace projeto {

/*
 * CentralizedQProtocol implements a protocol that recieves and sends DadcaMessages to simulate a
 * drone collecting data from sensors and sharing it with other drones. This protocol implements
 * the DADCA protocol.
 */
class CentralizedQProtocol : public CommunicationProtocolBase, public CentralizedQLearning::CentralizedQAgent
{
public:
    // Gets the agent's current state
    const LocalState& getAgentState() { return currentState; }

    // Applies a command to the agent
    virtual void applyCommand(const LocalControl& command);

    bool isReady() { return hasCompletedCommand; }

protected:
    CentralizedQLearning* learning;

    int agentId;
    bool hasCompletedCommand;
    LocalState currentState = {};
    Telemetry lastTelemetry;

    std::vector<Coord> tour;
    std::vector<double> tourPercentages;

protected:
    virtual void initialize(int stage) override;

    // Saves telemetry recieved by mobility
    virtual void handleTelemetry(Telemetry *telemetry) override;

    // Reacts to message recieved and updates payload accordingly
    virtual void handlePacket(Packet *pk) override;

    virtual void communicate(const LocalControl& control);
public:
    simsignal_t dataLoadSignalID;
};

} //namespace

#endif
