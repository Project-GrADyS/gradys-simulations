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

#include "CommunicationProtocolBase.h"

namespace projeto {

Define_Module(CommunicationProtocolBase);


void CommunicationProtocolBase::handleMessage(cMessage *msg) {
    Telemetry *telemetry = dynamic_cast<Telemetry *>(msg);
    if(telemetry != nullptr) {
        handleTelemetry(telemetry);
    }
    Packet *packet = dynamic_cast<Packet *>(msg);
    if(packet != nullptr) {
        handlePacket(packet);
    }

    cancelAndDelete(msg);
}

void CommunicationProtocolBase::sendCommand(MobilityCommand *order) {
    if(gate("mobilityGate$o")->isConnected()) {
        send(order, gate("mobilityGate$o"));
    }
}

void CommunicationProtocolBase::sendCommand(CommunicationCommand *order) {
    if(gate("communicationGate$o")->isConnected()) {
        send(order, gate("communicationGate$o"));
    }
}

void CommunicationProtocolBase::initiateTimeout(simtime_t duration) {
    timeoutEnd = simTime() + duration;
    timeoutSet = true;
}

bool CommunicationProtocolBase::isTimedout() {
    if(timeoutSet) {
        if(simTime() < timeoutEnd) {
            return true;
        }
        else {
            timeoutSet = false;
            return false;
        }
    } else {
        return false;
    }
}




} //namespace

