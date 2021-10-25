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

#include "SimpleTimerEnergy.h"
#include "../../communication/messages/internal/MobilityCommand_m.h"
#include "../../communication/messages/internal/Telemetry_m.h"

namespace projeto {

Define_Module(SimpleTimerEnergy);

void SimpleTimerEnergy::initialize(int stage) {
    if(stage == 0) {
        batteryDuration = par("batteryDuration");
        idleDuration = par("idleDuration");
        selfMessage = new cMessage();
    }
}

void SimpleTimerEnergy::handleMessage(cMessage *msg) {
    if(msg->isSelfMessage()) {
        MobilityCommand *returnCommand = new MobilityCommand();
        returnCommand->setCommandType(RETURN_TO_HOME);

        cGate *protocolGate = gate("mobilityGate$o");
        if(protocolGate->isConnected()) {
            send(returnCommand, protocolGate);
        }

        MobilityCommand *idleCommand = new MobilityCommand();
        idleCommand->setCommandType(IDLE_TIME);
        idleCommand->setParam1(idleDuration.dbl());

        if(protocolGate->isConnected()) {
            send(idleCommand, protocolGate);
        }
    } else {
        Telemetry *telemetry = dynamic_cast<Telemetry *>(msg);
        if(telemetry != nullptr) {
            if(telemetry->getDroneActivity() == NAVIGATING && !selfMessage->isScheduled()) {
                scheduleAt(simTime() + batteryDuration, selfMessage);
            }
        }
    }
}

SimpleTimerEnergy::~SimpleTimerEnergy() {
    cancelAndDelete(selfMessage);
}


} /* namespace projeto */
