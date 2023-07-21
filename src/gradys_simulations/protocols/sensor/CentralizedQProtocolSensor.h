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

#ifndef __gradys_simulations_CentralizedQProtocolSensor_H_
#define __gradys_simulations_CentralizedQProtocolSensor_H_

#include <omnetpp.h>
#include "../base/CommunicationProtocolBase.h"
#include "../auxiliary/CentralizedQLearning.h"
#include "../messages/internal/Telemetry_m.h"
#include "../messages/network/CentralizedQMessage_m.h"
#include "inet/common/geometry/common/Coord.h"

using namespace omnetpp;

namespace gradys_simulations {

class CentralizedQProtocolSensor : public CommunicationProtocolBase, public CentralizedQLearning::CentralizedQSensor
{
public:
    // Gets the number of awaiting packages the sensor has stored
    virtual uint32_t getAwaitingPackets() override { return awaitingPackets; };
    virtual uint32_t getMaxAwaitingPackets() override { return maxAwaitingPackets; };
    virtual bool hasBeenVisited() override { return visited; };
    virtual double getSensorPosition() override { return sensorPosition; };



protected:
    // Reference to the central learning module
    CentralizedQLearning* learning;

    // ID of this sensor
    int sensorId;

    // Position ([0,1]) of the sensor
    double sensorPosition;

    // Number of packets waiting to be picked up
    uint32_t awaitingPackets = 0;
    uint32_t maxAwaitingPackets = 0;

    bool visited = false;

    // Variables that control the generation of new packets. Packets are generated
    // every beta seconds. The generation timer controls this behavior, every time
    // the timer triggers a new packet is generated and stored.
    simtime_t beta;
    cMessage *generationTimer = new cMessage();

protected:
    // OMNET++ methods
    virtual void initialize(int stage) override;
    virtual void finish() override;
    virtual void handleMessage(cMessage *msg) override;

    // Reacts to message received from the communication module
    // and reacts accordingly.
    virtual void handlePacket(Packet *pk) override;
public:
    simsignal_t dataLoadSignalID;
};

} //namespace

#endif
