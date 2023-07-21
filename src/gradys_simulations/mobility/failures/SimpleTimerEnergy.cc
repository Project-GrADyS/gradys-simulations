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

#include "gradys_simulations/mobility/failures/SimpleTimerEnergy.h"

#include "gradys_simulations/protocols/messages/internal/MobilityCommand_m.h"

namespace gradys_simulations {

Define_Module(SimpleTimerEnergy);

void SimpleTimerEnergy::initialize(int stage) {
    if(stage == 0) {
        batteryRTLDuration = par("batteryRTLDuration");
        batteryShutdownDuration = par("batteryShutdownDuration");
        rechargeDuration = par("rechargeDuration");
        RTLMessage = new cMessage();
        shutdownMessage = new cMessage();
        isReturning = false;
    }
}

void SimpleTimerEnergy::handleMessage(cMessage *msg) {
    if(msg->isSelfMessage()) {
        if(msg == RTLMessage && !isReturning) {
            MobilityCommand *returnCommand = new MobilityCommand();
            returnCommand->setCommandType(RECHARGE);
            returnCommand->setParam1(rechargeDuration.dbl());

            cGate *protocolGate = gate("mobilityGate$o");
            if(protocolGate->isConnected()) {
                send(returnCommand, protocolGate);
            }
        } else if(msg == shutdownMessage && currentTelemetry.getCurrentCommand() != RECHARGE) {
            MobilityCommand *returnCommand = new MobilityCommand();
            returnCommand->setCommandType(FORCE_SHUTDOWN);

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


} /* namespace gradys_simulations */
