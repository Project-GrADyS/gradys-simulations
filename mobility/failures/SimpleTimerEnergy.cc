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

#include "../failures/SimpleTimerEnergy.h"

#include "../../communication/messages/internal/MobilityCommand_m.h"

namespace projeto {

Define_Module(SimpleTimerEnergy);

void SimpleTimerEnergy::initialize(int stage) {
    if(stage == 0) {
        batteryRTLDuration = par("batteryRTLDuration");
        batteryShutdownDuration = par("batteryShutdownDuration");
        idleDuration = par("idleDuration");
        RTLMessage = new cMessage();
        shutdownMessage = new cMessage();
        isReturning = false;
    }
}

void SimpleTimerEnergy::handleMessage(cMessage *msg) {
    if(msg->isSelfMessage()) {
        if(msg == RTLMessage && !isReturning) {
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
        } else if(msg == shutdownMessage && currentTelemetry.getCurrentCommand() != IDLE_TIME) {
            MobilityCommand *returnCommand = new MobilityCommand();
            returnCommand->setCommandType(SHUTDOWN);

            cGate *protocolGate = gate("mobilityGate$o");
            if(protocolGate->isConnected()) {
                send(returnCommand, protocolGate);
            }
        }
    } else {
        Telemetry *telemetry = dynamic_cast<Telemetry *>(msg);
        if(telemetry != nullptr) {
            currentTelemetry = *telemetry;

            if(isReturning && currentTelemetry.getDroneActivity() == NAVIGATING) {
                isReturning = false;

                if(shutdownMessage->isScheduled()) {
                    cancelEvent(shutdownMessage);
                }
            }

            if(telemetry->getDroneActivity() == NAVIGATING) {
                if(!RTLMessage->isScheduled()) {
                    scheduleAt(simTime() + batteryRTLDuration, RTLMessage);
                }
                if(!shutdownMessage->isScheduled()) {
                    scheduleAt(simTime() + batteryShutdownDuration, shutdownMessage);
                }
            }
            delete telemetry;
        }
    }
}

SimpleTimerEnergy::~SimpleTimerEnergy() {
    cancelAndDelete(RTLMessage);
    cancelAndDelete(shutdownMessage);
}


} /* namespace projeto */
