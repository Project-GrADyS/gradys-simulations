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

#ifndef __PROJETO_COMMUNICATIONPROTOCOLBASE_H_
#define __PROJETO_COMMUNICATIONPROTOCOLBASE_H_

#include <omnetpp.h>
#include "inet/common/packet/Packet.h"
#include "../../messages/internal/MobilityCommand_m.h"
#include "../../messages/internal/CommunicationCommand_m.h"
#include "../../messages/internal/Telemetry_m.h"

using namespace omnetpp;
using namespace inet;

namespace projeto {

/*
 *  Base class for the protocol classes
 *  The protocol is responsible for recieving messsages from both the mobility and communication
 *  modules and implementing movmement and communication protocols by responding to those modules.
 *
 *  The base implementation discards all messages and sends no responses.
 */

class CommunicationProtocolBase : public cSimpleModule
{
  protected:
    virtual void initialize(int stage) {};

    // Redirects message to the proper function
    virtual void handleMessage(cMessage *msg);

    // Handles package received from communication
    // This packet is a message that was sent to the drone
    virtual void handlePacket(Packet *pk) {};

    // Handles telemetry received from mobility
    // The mobility module exchanges mobility information in the form of telemetry
    virtual void handleTelemetry(Telemetry *telemetry) {};

    // Sends command to mobility
    virtual void sendCommand(MobilityCommand *order);
    // Sends command to communication
    virtual void sendCommand(CommunicationCommand *order);

    // Sets a timeout
    virtual void initiateTimeout(simtime_t duration);
    // Checks if the module is timed out
    virtual bool isTimedout();
  protected:
    simtime_t timeoutEnd;
    bool timeoutSet;
};

} //namespace

#endif
