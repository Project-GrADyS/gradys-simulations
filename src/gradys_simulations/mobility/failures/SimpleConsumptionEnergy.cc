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

#include "gradys_simulations/mobility/failures/SimpleConsumptionEnergy.h"

#include "gradys_simulations/protocols/messages/internal/MobilityCommand_m.h"

namespace gradys_simulations {

Define_Module(SimpleConsumptionEnergy);

void SimpleConsumptionEnergy::initialize(int stage)
{
    if(stage == 0) {
        batteryCapacity = par("batteryCapacity");
        batteryRTLThreshold = par("batteryRTLThreshold");
        batteryConsumption = par("batteryConsumption");
        rechargeDuration = par("rechargeDuration");
        selfMessage = new cMessage();

        currentConsumption = 0;
        scheduleAt(simTime() + 1, selfMessage);
    }
}

void SimpleConsumptionEnergy::handleMessage(cMessage *msg)
{
    if(msg->isSelfMessage()) {
        // Reseting after drone has recovered
        if(isReturning && currentTelemetry.getDroneActivity() == NAVIGATING) {
            isReturning = false;
            currentConsumption = 0;
        }

        if(currentTelemetry.getDroneActivity() != RECHARGING || currentTelemetry.getDroneActivity() != IDLE) {
            currentConsumption += (batteryConsumption / (60 * 60)) * 1000;
            EV_INFO << "Current battery usage: " << currentConsumption << "mAh/" << batteryCapacity << "mAh" << std::endl;
        }

        if(currentConsumption >= batteryRTLThreshold && !isReturning) {
           MobilityCommand *returnCommand = new MobilityCommand();
           returnCommand->setCommandType(RECHARGE);
           returnCommand->setParam1(rechargeDuration.dbl());

           cGate *protocolGate = gate("mobilityGate$o");
           if(protocolGate->isConnected()) {
               send(returnCommand, protocolGate);
           }

           isReturning = true;
        }

        if(currentConsumption >= batteryCapacity && currentTelemetry.getCurrentCommand() != RECHARGE) {
            MobilityCommand *shutdownCommand = new MobilityCommand();
            shutdownCommand->setCommandType(FORCE_SHUTDOWN);

            cGate *protocolGate = gate("mobilityGate$o");
            if(protocolGate->isConnected()) {
                send(shutdownCommand, protocolGate);
            }
        }


        scheduleAt(simTime() + 1, selfMessage);
   } else {
       Telemetry *telemetry = dynamic_cast<Telemetry *>(msg);
       if(telemetry != nullptr) {
               currentTelemetry = *telemetry;
           }
           delete telemetry;
       }
    }

} //namespace
