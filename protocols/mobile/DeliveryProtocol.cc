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

#include "DeliveryProtocol.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/packet/Packet.h"
#include "../messages/network/DeliveryMessage_m.h"
#include "../messages/internal/MobilityCommand_m.h"

namespace projeto {
Define_Module(DeliveryProtocol);

void DeliveryProtocol::initialize(int stage) {
    // Loading the parameter timeoutDuration
    timeoutDuration = par("timeoutDuration");

    // Emits the first dataLoad signal with value 0
    emit(registerSignal("dataLoad"), 0);

    if (stage == 0) {
        scheduleAt(simTime() + uniform(0.1, 0.5), timer);
    }

    WATCH(target);
    WATCH(index);
    WATCH(hasVisitedTarget);
    WATCH(round);
}

void DeliveryProtocol::handleMessage(cMessage *message) {
    if (message == timer) {
        if(isTraveling) {
            scheduleAt(simTime() + uniform(0.1, 0.5), timer);
            return;
        }

        DeliveryMessage* payload = new DeliveryMessage();
        payload->setRound(round);

        if (round % 10 == index) {
            if (!hasWaited) {
                scheduleAt(simTime() + 1, timer);
                hasWaited = true;
                return;
            }
            payload->setType(COMMAND);

            if(!hasVisitedTarget) {
                received.push_back(target);
            }

            if (received.size() > 0) {
                int targetMode = -1;
                int targetModeCount = -1;
                for (int candidate = 0; candidate < targets.size(); candidate++) {
                    int count = 0;
                    for (int t: received) {
                        if(t == candidate) {
                            count++;
                        }
                    }
                    if (count > targetModeCount) {
                        targetModeCount = count;
                        targetMode = candidate;
                    }
                }
                payload->setLocation(targetMode);
            } else {
                return;
            }
            if (payload->getLocation() == target) {
                hasVisitedTarget = true;
            }
            hasWaited = false;
            std::cout << std::endl << std::endl;
            std::cout << payload->getLocation() << std::endl;
            received.clear();
            round++;
            goToTarget(payload->getLocation());

            if (timer->isScheduled()) {
                cancelEvent(timer);
            }
            scheduleAt(simTime() + uniform(0.1, 0.5), timer);
        } else {
            scheduleAt(simTime() + 2, timeout);

            if(hasVisitedTarget) {
                return;
            }

            payload->setType(SHARE);
            payload->setLocation(target);
        }

        CommunicationCommand *command = new CommunicationCommand();
        command->setCommandType(CommunicationCommandType::SEND_MESSAGE);
        command->setPayloadTemplate(payload);
        sendCommand(command);
    } else if (message == timeout) {
        round++;
        scheduleAt(simTime() + uniform(0.1, 0.5), timer);
    } else {
        CommunicationProtocolBase::handleMessage(message);
    }
}


void DeliveryProtocol::goToTarget(int target) {
    MobilityCommand *resume = new MobilityCommand();
    resume->setCommandType(WAKE_UP);
    sendCommand(resume);

    auto module = getModuleByPath("coordinateSystem");
    IGeographicCoordinateSystem* coordinateSystem = dynamic_cast<IGeographicCoordinateSystem*>(module);

    Coord coord = coordinateSystem->computeSceneCoordinate(targets[target]);

    MobilityCommand *command = new MobilityCommand();
    command->setCommandType(GOTO_COORDS);
    command->setParam1(coord.x);
    command->setParam2(coord.y);
    command->setParam3(coord.z);
    command->setParam4(0);
    command->setParam5(0);

    sendCommand(command);
    isTraveling=true;
}

void DeliveryProtocol::handlePacket(Packet* pk) {
    auto message = pk->peekAtBack<DeliveryMessage>(B(7), 1);
    if(message == nullptr) {
        return;
    }

    if (message->getRound() < round) {
        return;
    } else if (message->getRound() > round) {
        round = message->getRound();
    }


    if (message->getType() == COMMAND) {
        cancelEvent(timeout);
        std::cout << message->getLocation() << std::endl;
        if (target == message->getLocation()) {
            hasVisitedTarget = true;
        }
        received.clear();
        goToTarget(message->getLocation());
        if (timer->isScheduled()) {
            cancelEvent(timer);
        }
        scheduleAt(simTime() + uniform(0.1, 0.5), timer);
        round++;
    } else {
        received.push_back(message->getLocation());
    }
}



void DeliveryProtocol::handleTelemetry(Telemetry *telemetry) {
    isTraveling = telemetry->getCurrentCommand() == GOTO_COORDS;
    if (!isTraveling && telemetry->getCurrentCommand() != FORCE_SHUTDOWN) {
        MobilityCommand *command = new MobilityCommand();
        command->setCommandType(FORCE_SHUTDOWN);
        sendCommand(command);
    }
}

void DeliveryProtocol::finish() {
    CommunicationProtocolBase::finish();
}
} /* namespace projeto */
