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

#include "RandomFailureGenerator.h"

#include "gradys_simulations/protocols/messages/internal/MobilityCommand_m.h"

namespace gradys_simulations {

Define_Module(RandomFailureGenerator);

enum TimerMessageKinds {
    INTERVAL,
    WAKE
};

void RandomFailureGenerator::initialize()
{
    failureStart = par("failureStart");
    failureMinimumInterval = par("failureMinimumInterval");
    failureRollTime = par("failureRollTime");
    failureChance = par("failureChance");
    failureDuration = par("failureDuration");

    timerMessage = new cMessage();
    timerMessage->setKind(INTERVAL);
    scheduleAt(simTime() + 1, timerMessage);
}

void RandomFailureGenerator::handleMessage(cMessage *msg)
{
    if(msg->isSelfMessage()) {
        switch(msg->getKind()) {
            case INTERVAL:
            {
                float rng = uniform(0, 1);

                EV_INFO << "Rolled " << rng << std::endl;
                if(rng - failureChance <= 0) {
                    EV_INFO << "Triggering failure" << std::endl;;
                    MobilityCommand *shutdownCommand = new MobilityCommand();
                    shutdownCommand->setCommandType(FORCE_SHUTDOWN);

                    cGate *protocolGate = gate("mobilityGate$o");
                    if(protocolGate->isConnected()) {
                        send(shutdownCommand, protocolGate);
                    }

                    msg->setKind(WAKE);
                    scheduleAt(simTime() + failureDuration, msg);
                } else {
                    scheduleAt(simTime() + failureRollTime, msg);
                }
                break;
            }
            case WAKE:
            {
                EV_INFO << "Waking up vehicle" << std::endl;;
                MobilityCommand *wakeCommand = new MobilityCommand();
                wakeCommand->setCommandType(WAKE_UP);

                cGate *protocolGate = gate("mobilityGate$o");
                if(protocolGate->isConnected()) {
                    send(wakeCommand, protocolGate);
                }

                msg->setKind(INTERVAL);
                scheduleAt(simTime() + (failureMinimumInterval > failureRollTime ? failureMinimumInterval : failureRollTime), msg);
            }
        }
    }
}

} //namespace
