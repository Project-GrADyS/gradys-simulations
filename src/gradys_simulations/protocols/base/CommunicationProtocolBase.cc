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

namespace gradys_simulations {

Define_Module(CommunicationProtocolBase);


void CommunicationProtocolBase::handleMessage(cMessage *msg) {
    Telemetry *telemetry = dynamic_cast<Telemetry *>(msg);
    if(telemetry != nullptr) {
        handleTelemetry(telemetry);
    } else {
        Packet *packet = dynamic_cast<Packet *>(msg);
        if(packet != nullptr) {
            handlePacket(packet);
        }
    }
    delete msg;
}

void CommunicationProtocolBase::sendCommand(MobilityCommand *order, int gateIndex) {
    if(gateIndex == -1) {
        for(int i=0;i<gateSize("mobilityGate");i++) {
            if(gate("mobilityGate$o", i)->isConnected()) {
                send(order->dup(), gate("mobilityGate$o", i));
            }
        }
        delete order;
    } else {
        if(gate("mobilityGate$o", gateIndex)->isConnected()) {
            send(order, gate("mobilityGate$o", gateIndex));
        } else {
            delete order;
        }
    }
}

void CommunicationProtocolBase::sendCommand(PythonMobilityCommand *order, int gateIndex) {
    if(gateIndex == -1) {
        for(int i=0;i<gateSize("mobilityGate");i++) {
            if(gate("mobilityGate$o", i)->isConnected()) {
                send(order->dup(), gate("mobilityGate$o", i));
            }
        }
        delete order;
    } else {
        if(gate("mobilityGate$o", gateIndex)->isConnected()) {
            send(order, gate("mobilityGate$o", gateIndex));
        } else {
            delete order;
        }
    }
}

void CommunicationProtocolBase::sendCommand(CommunicationCommand *order, int gateIndex) {
    if(gateIndex == -1) {
        for(int i=0;i<gateSize("communicationGate");i++) {
            if(gate("communicationGate$o", i)->isConnected()) {
                send(order->dup(), gate("communicationGate$o", i));
            }
        }
        delete order;
    } else {
        if(gate("communicationGate$o", gateIndex)->isConnected()) {
            send(order, gate("communicationGate$o", gateIndex));
        } else {
            delete order;
        }
    }
}

void CommunicationProtocolBase::initiateTimeout(simtime_t duration) {
    if(duration > 0) {
        timeoutEnd = simTime() + duration;
        timeoutSet = true;
    }
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

