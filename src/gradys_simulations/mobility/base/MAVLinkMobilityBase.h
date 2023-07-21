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

#ifndef __RT_EXPERIMENT_COMMUNICATIONTESTMODULE_H_
#define __RT_EXPERIMENT_COMMUNICATIONTESTMODULE_H_

#include <omnetpp.h>
#include <queue>
#include <functional>
#include <memory>
#include "inet/mobility/base/MovingMobilityBase.h"
#include "inet/common/geometry/common/GeographicCoordinateSystem.h"
#include "inet/common/scheduler/RealTimeScheduler.h"
#include "mavlink/ardupilotmega/mavlink.h"
#include "gradys_simulations/utils/subprocess/process.h"
#include "gradys_simulations/mobility/base/MAVLinkInstruction.h"
#include "gradys_simulations/utils/VehicleTypes.h"

#ifdef _WIN32
#include <winsock2.h>
#endif

using namespace inet;

namespace gradys_simulations {

class MAVLinkMobilityBase : public MovingMobilityBase, public RealTimeScheduler::ICallback
{
  protected:
    virtual ~MAVLinkMobilityBase();
    virtual void initialize(int stage) override;
    // Freezes initialization until the vehicle is pre-arm ready
    virtual void waitUntilReady();
    // Starts a SITL simulator instance
    virtual void startSimulator();
    // Opens the socket to the SITL instance
    virtual void openSocket();

    virtual void handleMessage(cMessage *msg) override;

    virtual void finish() override;
    virtual void move() override;
    virtual void orient() override;

    // This function is called by the RealTimeScheduler when a message is received on the open socket.
    // It reads the message and translates it to a MAVLink message that is sent to receibeTelemetry.
    // It should probably not be overriden.
    virtual bool notify(int fd) override;

    // Callback function called when a message is received from the simulated SITL instance. The default behaviour is to
    // check if the message completes the condition of the active MAVLinkInstruction (if one exists) and to call the next
    // MAVLinkInstruction if the current one is done.
    virtual void receiveTelemetry(mavlink_message_t const& message);

    // Performs initial setup on the vehicle. This includes setting update rate and setting home to current position
    virtual void performInitialSetup();
    // Receives telemetry and updates the vehicle's position
    virtual void updatePosition(const mavlink_message_t& message);
    // Queues instruction
    virtual void queueMessage(mavlink_message_t message, Condition condition = {}, simtime_t timeout = -1, int retries = 0, std::string label = "");
    // Queues instruction
    virtual void queueInstruction(std::shared_ptr<Instruction> instruction);
    // Queues list of instructions
    virtual void queueInstructions(std::vector<std::shared_ptr<Instruction>> instructions);

    virtual int queueSize() { return instructionQueue.size(); }
    virtual void clearQueue();

    // Starts first instruction in queue
    virtual void nextMessage();
    // Gets first instruction from queue if the active instruction is completed
    virtual void nextMessageIfReady();
    // Sends the current active instruction
    virtual bool sendActiveMessage();
    // Sends a message and returns if successful
    virtual bool sendMessage(const mavlink_message_t& message, bool shouldRetry, int &currentTries, int maxRetries);

    mavlink_message_t getActiveMessage() { return (activeInstruction != nullptr) ? activeInstruction->message : mavlink_message_t{}; };
    Condition getActiveCondition() { return (activeInstruction != nullptr) ? activeInstruction->condition : Condition{}; };
    simtime_t getActiveTimeout() { return (activeInstruction != nullptr) ? activeInstruction->timeout : 0; };
    int getActiveRetries() { return (activeInstruction != nullptr) ? activeInstruction->retries : 0; };
    bool getActiveCompleted() { return (activeInstruction != nullptr) ? activeInstruction->completed : true; };
    std::string getActiveLabel() { return (activeInstruction != nullptr) ? activeInstruction->label : ""; };
    Coord getCurrentCoord() { return (activeInstruction != nullptr) ? currentPosition : Coord{}; }


    enum CommunicationSelfMessages {
        TIMEOUT,
        HEARTBEAT,
        UPDATE
    };

  protected:
    int basePort;
#ifdef _WIN32
    SOCKET socketFd;
#else
    int socketFd;
#endif

    char buf[256];
    VehicleType vehicleType;
    RealTimeScheduler *rtScheduler;
    uint8_t systemId, componentId, targetSystem, targetComponent;
    Coord currentPosition;
    Quaternion currentOrientation;
    IGeographicCoordinateSystem *coordinateSystem;
    TinyProcessLib::Process* simulatorProcess;
    std::string paramPath;
    std::string copterSimulatorPath;
    std::string planeSimulatorPath;
    std::string roverSimulatorPath;

    std::string executionPath;
  private:
    std::queue<std::shared_ptr<Instruction>> instructionQueue;

    std::shared_ptr<Instruction> activeInstruction = nullptr;
    int activeInstructionTries = 0;

    cMessage *timeoutMessage = new cMessage("MAVLinkMobilityBaseMessage", CommunicationSelfMessages::TIMEOUT);
    cMessage *heartbeatMessage = new cMessage("MAVLinkMobilityBaseMessage", CommunicationSelfMessages::HEARTBEAT);
    cMessage *updateMessage = new cMessage("MAVLinkMobilityBaseMessage", CommunicationSelfMessages::UPDATE);
    mavlink_message_t heartbeat = {};
};


}
#endif
