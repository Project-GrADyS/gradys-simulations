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

#ifndef __gradys_simulations_DadcaProtocol_H_
#define __gradys_simulations_DadcaProtocol_H_

#include <omnetpp.h>
#include "../base/CommunicationProtocolBase.h"
#include "../messages/network/DadcaMessage_m.h"
#include "inet/common/geometry/common/Coord.h"

using namespace omnetpp;

namespace gradys_simulations {

enum CommunicationStatus { FREE=0, REQUESTING=1, PAIRED=2, COLLECTING=3, PAIRED_FINISHED=4 };

/*
 * DadcaProtocol implements a protocol that recieves and sends DadcaMessages to simulate a
 * drone collecting data from sensors and sharing it with other drones. This protocol implements
 * the DADCA protocol.
 */
class DadcaProtocol : public CommunicationProtocolBase
{
    protected:
        simtime_t timeoutDuration;

        // DADCA variables
        // Current tour recieved from telemetry
        std::vector<Coord> tour;

        int leftNeighbours = 0;
        int rightNeighbours = 0;

        // Communication status variable
        CommunicationStatus communicationStatus = FREE;


        // Current target
        int tentativeTarget = -1;
        // Previous target
        int lastTarget = -1;
        // Name of the current target (for addressing purposes)
        std::string tentativeTargetName;

        // Current imaginary data being carried
        int currentDataLoad=0;

        // Maximum number of collected packets
        int packetLimit;
        // Stable data load to prevent data loss during pairing
        int stableDataLoad=currentDataLoad;

        // Last telemetry package recieved
        Telemetry currentTelemetry = Telemetry();
        Telemetry lastStableTelemetry = Telemetry();

        DadcaMessage lastPayload = DadcaMessage();

    protected:
        virtual void initialize(int stage) override;

        // Saves telemetry recieved by mobility
        virtual void handleTelemetry(gradys_simulations::Telemetry *telemetry) override;
        // Reacts to message recieved and updates payload accordingly
        virtual void handlePacket(Packet *pk) override;
        // Checks if timeout has finished and resets parameters if it has
        virtual bool isTimedout() override;
        // Resets parameters
        virtual void resetParameters();
    private:
        // Sends sequence of orders that defines a rendevouz point, navigates
        // to it and reverses
        virtual void rendevouz();

        // Updates payload that communication will send
        virtual void updatePayload();
        virtual void setTarget(const char *target);
    public:
        simsignal_t dataLoadSignalID;
};

} //namespace

#endif
