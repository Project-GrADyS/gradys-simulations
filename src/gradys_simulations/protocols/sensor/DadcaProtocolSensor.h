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

#ifndef __gradys_simulations_DadcaProtocolSensor_H_
#define __gradys_simulations_DadcaProtocolSensor_H_

#include <omnetpp.h>
#include "../base/CommunicationProtocolBase.h"
#include "../messages/network/DadcaMessage_m.h"
#include "inet/common/geometry/common/Coord.h"

using namespace omnetpp;

namespace gradys_simulations {

/*
 * DadcaProtocol implements a protocol that recieves and sends DadcaMessages to simulate a
 * drone collecting data from sensors and sharing it with other drones. This protocol implements
 * the DADCA protocol.
 */
class DadcaProtocolSensor : public CommunicationProtocolBase
{
    protected:
        // Current target
        int tentativeTarget = -1;
        // Name of the current target (for addressing purposes)
        std::string tentativeTargetName;

        DadcaMessage lastPayload = DadcaMessage();

        cMessage *generationTimer = new cMessage();
        simtime_t generationInterval;
        int awaitingPackets = 0;
    protected:
        virtual void initialize(int stage) override;

        virtual void handleMessage(cMessage *msg) override;

        // Sensor does not recieve telemetry
        virtual void handleTelemetry(gradys_simulations::Telemetry *telemetry) override { return; };
        // Reacts to message recieved and updates payload accordingly
        virtual void handlePacket(Packet *pk) override;
    private:
        // Updates payload that communication will send
        virtual void updatePayload(const char *target);
    public:
        simsignal_t dataLoadSignalID;
};

} //namespace

#endif
