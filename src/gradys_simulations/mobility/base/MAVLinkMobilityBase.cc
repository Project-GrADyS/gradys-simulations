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

#include "MAVLinkMobilityBase.h"
#include "gradys_simulations/utils/TelemetryConditions.h"
#include <chrono>
#include <thread>

#ifdef _WIN32
#define SOCKET_ERROR_CODE WSAGetLastError()
#define SOCKET_EMPTY_ERROR WSAEWOULDBLOCK
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define SOCKET_ERROR_CODE errno
#define SOCKET_EMPTY_ERROR EAGAIN
#endif

using namespace omnetpp;
using namespace inet;

namespace gradys_simulations {


Define_Module(MAVLinkMobilityBase);

static int lastId = 1;

void MAVLinkMobilityBase::initialize(int stage)
{
    MovingMobilityBase::initialize(stage);
    if (stage == INITSTAGE_LOCAL) {
        targetSystem = (par("targetSystem").intValue() == -1) ? lastId++ : par("targetSystem");
        targetComponent = par("targetComponent");
        vehicleType = static_cast<VehicleType>(par("vehicleType").intValue());
        coordinateSystem = getModuleFromPar<IGeographicCoordinateSystem>(par("coordinateSystemModule"), this);

        rtScheduler = dynamic_cast<inet::RealTimeScheduler *>(getSimulation()->getScheduler());

        if(rtScheduler == nullptr) {
            EV_ERROR << "The scheduler module is not inet::RealTimeScheduler. This means that the simulation will have no "
                    "guarantee of synchronicity. You will be responsible for keeping the simulation time at a rate of 1/1 to"
                    "real time. To do this the playback speed should be \"1 / mobility animation speed\"." << std::endl;
        }

        basePort = par("basePort");
        systemId = par("systemId");
        componentId = par("componentId");
        paramPath = par("paramPath").stdstringValue();
        copterSimulatorPath = par("copterSimulatorPath").stdstringValue();
        planeSimulatorPath = par("planeSimulatorPath").stdstringValue();
        roverSimulatorPath = par("roverSimulatorPath").stdstringValue();
        executionPath = "gradys_simulations-i";
        executionPath += std::to_string(+targetSystem);
        startSimulator();
        openSocket();
        performInitialSetup();


        scheduleAt(simTime() + s(1).get(), heartbeatMessage);
    }
    if (stage == 1) {
        if(par("waitUntilReady")) {
            waitUntilReady();
        }
    }
}


void MAVLinkMobilityBase::waitUntilReady() {
    int length;

    // Prematurely and forcefully sending the message rate for the EKF_STATUS_REPORT message.
    // This message is used to determine if a vehicle is ready to arm. It is also sent by
    // performInitialSetup but in that function it is queued, and the queue won't proceed
    // until the simulation is started, which only happens after this function returns.
    mavlink_message_t message = {};
    mavlink_command_long_t cmd = {};
    cmd.command = MAV_CMD_SET_MESSAGE_INTERVAL;
    cmd.confirmation = 0;
    cmd.param1 = MAVLINK_MSG_ID_EKF_STATUS_REPORT;
    cmd.param2 = 2000000; // 0.5 Hz
    cmd.param7 = 0;
    cmd.target_component = targetComponent;
    cmd.target_system = targetSystem;
    mavlink_msg_command_long_encode(systemId, componentId, &message, &cmd);
    int retries = 0;
    sendMessage(message, false, retries, 1);
    std::cout << "(" << +targetSystem << ") Forcefully setting interval for EKF status reports" << std::endl;

    Condition preReadyCondition = TelemetryConditions::getCheckPreArm(targetSystem);
    EV_INFO << "Waiting until simulator is ready" << std::endl;
    while(true) {
        if((length = recv(socketFd, buf, 256, 0)) == SOCKET_ERROR) {
            int error = SOCKET_ERROR_CODE;
            // Blocking errors are normal when no messages are present in a non-blocking socket
            if(error == SOCKET_EMPTY_ERROR) {
                // Sleeps to save on processing power while waiting
                std::cout << "(" << +targetSystem << ") No messages in socket, sleeping..." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }

            std::cout << "(" << +targetSystem << ") Error receiving message, code: " << error << std::endl;
            return;
        }

        mavlink_status_t status = {};
        mavlink_message_t msg = {};

        for (int i = 0; i < length; ++i)
        {
            if (mavlink_parse_char(MAVLINK_COMM_0, buf[i], &msg, &status))
            {
                std::cout << "(" << +targetSystem << ") Received message id " << +msg.msgid << " while waiting until simulator is ready" << std::endl;
                if(preReadyCondition(msg)) {
                    EV_INFO << "Vehicle ready to arm" << std::endl;
                    return;
                }
            }
        }
    }
}

void MAVLinkMobilityBase::startSimulator() {
    std::string command;

    switch(vehicleType) {
    case COPTER:
        command += copterSimulatorPath;
        command += " -M quad -w --defaults " + paramPath;
        break;
    case PLANE:
        command += planeSimulatorPath;
        command += " -M plane -w --defaults " + paramPath;
        break;
    case ROVER:
        command += roverSimulatorPath;
        command += " -M rover -w --defaults " + paramPath;
    }

    command += " --wipe ";
    command += " --base-port ";
    command += std::to_string(basePort + (targetSystem * 10));
    command += " --sysid ";
    command += std::to_string(+targetSystem);
    command += " --home ";
    command += std::to_string(par("initialLatitude").doubleValue()) + "," +
               std::to_string(par("initialLongitude").doubleValue()) + "," +
               std::to_string(par("initialAltitude").doubleValue()) + ",0";

    EV_INFO << "Starting simulator with command: " << command << std::endl;

    // Executing each instance in it's own directory to avoid conflicts
#ifdef _WIN32
    // Creating a temporary directory
    _mkdir(executionPath.c_str());
#else
    // Creating a temporary directory
    mkdir(executionPath.c_str(), 0777);
#endif
    TinyProcessLib::Process *process = new TinyProcessLib::Process(command, executionPath);
    simulatorProcess = process;
}

void MAVLinkMobilityBase::openSocket()
{
    struct sockaddr_in server;
#ifdef _WIN32
    WSADATA wsa;

    EV_INFO << "Initialising Winsock..." << std::endl;
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
    {
        EV_ERROR << "Failed. Error Code : %d\n",SOCKET_ERROR_CODE;
        throw std::runtime_error("Error initializing socket library");
    }

    EV_INFO << "Initialised" << std::endl;
#endif
    //Create a socket
    // TODO: This should be a UDP socket if we are connecting to a real vehicle
    int fd;
    if((fd = socket(AF_INET , SOCK_STREAM , 0)) == INVALID_SOCKET)
    {
        EV_ERROR << "Could not create socket : " << SOCKET_ERROR_CODE << std::endl;
        throw std::runtime_error("Error creating socket");
    }

    socketFd = fd;

    EV_INFO << "Socket created" << std::endl;

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons( basePort + (targetSystem * 10) );

    // Bind
    // Retries 5 times with sleeps in between. This prevents the socket from trying to connect while
    // the simulator is still starting
    int tries = 0;
    while (true) {
        if( connect(fd ,(struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR)
        {
            if(tries > 5) {
                EV_ERROR << "Connect failed with error code : " << SOCKET_ERROR_CODE << std::endl;
                throw std::runtime_error("Error connecting to socket");
            } else {
                tries++;
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        } else {
            break;
        }
    }
    EV_INFO << "Socket bound" << std::endl;


    #ifdef _WIN32
       unsigned long mode = 1;
       if(ioctlsocket(fd, FIONBIO, &mode) == SOCKET_ERROR) {
           throw std::runtime_error("Error setting socket to non blocking");
       }
    #else
       int flags = fcntl(fd, F_GETFL, 0);
       if (flags == -1) throw;
       flags = (flags | O_NONBLOCK);
       if(fcntl(fd, F_SETFL, flags) == SOCKET_ERROR) {
           throw std::runtime_error("Error setting socket to non blocking");
       }
    #endif
    EV_INFO << "Socket set to non-blocking" << std::endl;

    // If the simulation has a RealTimeScheduler than uses it
    // to handle socket consumption, otherwise this will happen
    // in the HEARTBEAT event
    if(rtScheduler != nullptr) {
        rtScheduler->addCallback(fd, this);
    }
}


bool MAVLinkMobilityBase::notify(int incoming) {
    Enter_Method_Silent();
    EV_DETAIL << "Notified" << std::endl;

    if(incoming == socketFd) {
        int length;

        do {
            if((length = recv(incoming, buf, 256, 0)) == SOCKET_ERROR) {
                int error = SOCKET_ERROR_CODE;
                // Blocking errors are normal when no messages are present in a non-blocking socket
                if(error == SOCKET_EMPTY_ERROR) {
                    EV_DEBUG << "Received SOCKET_EMPTY_ERROR, the socket is empty." << std::endl;
                    return true;
                }
                EV_ERROR << "Error receiving message, code: " << error << std::endl;
                return false;
            }
            EV_DETAIL << "Received " << length << " bytes." << std::endl;

            mavlink_status_t status = {};
            mavlink_message_t msg = {};

            for (int i = 0; i < length; ++i)
            {
                if (mavlink_parse_char(MAVLINK_COMM_0, buf[i], &msg, &status))
                {
                    updatePosition(msg);
                    receiveTelemetry(msg);
                }
            }
        } while(length > 0);
        return true;
    }
    return false;
}

void MAVLinkMobilityBase::handleMessage(cMessage *msg) {
    Enter_Method_Silent();
    if(msg->isSelfMessage()) {
        if(strcmp(msg->getName(), "MAVLinkMobilityBaseMessage") == 0) {
            switch(msg->getKind()) {
                case CommunicationSelfMessages::TIMEOUT:
                    if(activeInstructionTries < getActiveRetries()) {
                        EV_WARN << "Timeout reached" << std::endl;
                        activeInstructionTries++;
                        sendActiveMessage();

                        // Setting up timeout again if there are any retries left
                        if(getActiveTimeout() > 0) {
                            scheduleAt(simTime() + getActiveTimeout(), timeoutMessage);
                        }
                    } else {
                        EV_WARN << "Max retries reached." << activeInstructionTries << std::endl;
                        nextMessage();
                    }
                    return;
                case CommunicationSelfMessages::HEARTBEAT:
                    if(heartbeat.msgid != MAVLINK_MSG_ID_HEARTBEAT) {
                        mavlink_heartbeat_t msg = {};
                        msg.autopilot = MAV_AUTOPILOT_INVALID;
                        msg.base_mode = 0;
                        msg.custom_mode = 0;
                        msg.mavlink_version = 3;
                        msg.system_status = 0;
                        msg.type = MAV_TYPE_GCS;

                        mavlink_msg_heartbeat_encode(systemId, componentId, &heartbeat, &msg);
                    }

                    int tries = 0;
                    sendMessage(heartbeat, false, tries, 0);

                    scheduleAt(simTime() + s(1).get(), heartbeatMessage);
            }
        }
    }
    MovingMobilityBase::handleMessage(msg);

}



void MAVLinkMobilityBase::performInitialSetup() {
    mavlink_command_long_t cmd = {};
    cmd.command = MAV_CMD_SET_MESSAGE_INTERVAL;
    cmd.confirmation = 0;
    cmd.param1 = MAVLINK_MSG_ID_GLOBAL_POSITION_INT;
    cmd.param2 = updateInterval.inUnit(SimTimeUnit::SIMTIME_US);
    cmd.param7 = 0;
    cmd.target_component = targetComponent;
    cmd.target_system = targetSystem;

    mavlink_message_t message = {};
    mavlink_msg_command_long_encode(systemId, componentId, &message, &cmd);
    queueMessage(message,
            TelemetryConditions::getCheckCmdAck(systemId, componentId, MAV_CMD_SET_MESSAGE_INTERVAL, targetSystem),
            15, 5, "Setting stream rate for position messages");

    cmd = {};
    cmd.command = MAV_CMD_SET_MESSAGE_INTERVAL;
    cmd.confirmation = 0;
    cmd.param1 = MAVLINK_MSG_ID_ATTITUDE;
    cmd.param2 =  updateInterval.inUnit(SimTimeUnit::SIMTIME_US);
    cmd.param7 = 0;
    cmd.target_component = targetComponent;
    cmd.target_system = targetSystem;
    mavlink_msg_command_long_encode(systemId, componentId, &message, &cmd);
    queueMessage(message,
            TelemetryConditions::getCheckCmdAck(systemId, componentId, MAV_CMD_SET_MESSAGE_INTERVAL, targetSystem),
            15, 5, "Setting stream rate for attitude messages");

    cmd = {};
    cmd.command = MAV_CMD_SET_MESSAGE_INTERVAL;
    cmd.confirmation = 0;
    cmd.param1 = MAVLINK_MSG_ID_EKF_STATUS_REPORT;
    cmd.param2 = 2000000; // 0.5 Hz
    cmd.param7 = 0;
    cmd.target_component = targetComponent;
    cmd.target_system = targetSystem;
    mavlink_msg_command_long_encode(systemId, componentId, &message, &cmd);
    queueMessage(message,
            TelemetryConditions::getCheckCmdAck(systemId, componentId, MAV_CMD_SET_MESSAGE_INTERVAL, targetSystem),
            15, 5, "Setting stream rate for ekf reports");

    cmd = {};
    cmd.command = MAV_CMD_SET_MESSAGE_INTERVAL;
    cmd.confirmation = 0;
    cmd.param1 = MAVLINK_MSG_ID_HEARTBEAT;
    cmd.param2 = 2000000; // 0.5 Hz
    cmd.param7 = 0;
    cmd.target_component = targetComponent;
    cmd.target_system = targetSystem;
    mavlink_msg_command_long_encode(systemId, componentId, &message, &cmd);
    queueMessage(message,
            TelemetryConditions::getCheckCmdAck(systemId, componentId, MAV_CMD_SET_MESSAGE_INTERVAL, targetSystem),
            15, 5, "Setting stream rate for heart beats");
}

void MAVLinkMobilityBase::queueMessage(mavlink_message_t message, Condition condition, simtime_t timeout, int retries, std::string label) {
    instructionQueue.push(std::make_shared<Instruction>(message, condition, timeout, retries, label));
}

void MAVLinkMobilityBase::queueInstruction(std::shared_ptr<Instruction> instruction) {
    instructionQueue.push(instruction);
}

void MAVLinkMobilityBase::queueInstructions(std::vector<std::shared_ptr<Instruction>> instructions) {
    for (std::shared_ptr<Instruction> instruction : instructions) {
        instructionQueue.push(instruction);
    }
}

void MAVLinkMobilityBase::nextMessage() {
    EV_DETAIL << "Proceeding to next message" << std::endl;
    Enter_Method_Silent();
    cancelEvent(timeoutMessage);

    if(instructionQueue.size() > 0) {
        activeInstructionTries = 0;
        activeInstruction = instructionQueue.front();
        instructionQueue.pop();

        if(!getActiveLabel().empty()) {
            EV_INFO << "Setting \"" << getActiveLabel() << "\" instruction to active." << std::endl;
        }

        sendActiveMessage();


        // Setting up timeout
        if(getActiveTimeout() > 0) {
            scheduleAt(simTime() + getActiveTimeout(), timeoutMessage);
        }
    }
}

void MAVLinkMobilityBase::nextMessageIfReady() {
    if(!getActiveCondition() || getActiveCompleted()) {
        nextMessage();
    }
}

void MAVLinkMobilityBase::clearQueue() {
    EV_DETAIL << "Clearing message queue" << std::endl;

    activeInstruction = nullptr;
    instructionQueue = {};
    activeInstructionTries = 0;
}

bool MAVLinkMobilityBase::sendActiveMessage() {
    if(activeInstruction != nullptr) {
        if(getActiveLabel().empty()) {
            EV_INFO << "(" << +targetSystem << ") Sending message: " << getActiveMessage().msgid << std::endl;
        }
        else {
            EV_INFO << "(" << +targetSystem << ") Sending message: " << getActiveLabel() << " - " << getActiveMessage().msgid << std::endl;
        }
        bool success = sendMessage(getActiveMessage(), getActiveTimeout() == 0,activeInstructionTries, getActiveRetries());
        if(!success && getActiveTimeout() == 0) {
            nextMessage();
            return false;
        }
        return success;
    }
    return false;
}

bool MAVLinkMobilityBase::sendMessage(const mavlink_message_t& message, bool shouldRetry, int &currentTries, int maxRetries) {
    int length = mavlink_msg_to_send_buffer((uint8_t*) buf, &message);
    static const int c = sizeof(sockaddr_in);

    do {
        if(currentTries >= 1) {
            EV_WARN << "RETRY " << currentTries << std::endl;
        }



        if(::send(socketFd, buf, length, 0) == SOCKET_ERROR) {
            EV_WARN << "Failed to send message: " << SOCKET_ERROR_CODE << std::endl;
            if (shouldRetry == true) {
                currentTries++;
            } else {
                return false;
            }
        } else {
            EV_DEBUG << "Message sent: " << message.msgid << std::endl;
            return true;
        }
    } while(currentTries < maxRetries);
    EV_WARN << "Max retries reached: " << activeInstructionTries << std::endl;
    return false;
}

void MAVLinkMobilityBase::move() {
    // If the RealTimeScheduler is not used, notifies for socket consumption
    // on the move event
    if(rtScheduler == nullptr) {
        notify(socketFd);
    }

    lastVelocity = (currentPosition - lastPosition) * (1/(simTime() - lastUpdate));
    lastPosition = currentPosition;
}

void MAVLinkMobilityBase::orient() {
    lastOrientation = currentOrientation;
}


void MAVLinkMobilityBase::updatePosition(const mavlink_message_t& msg) {
    // Updating vehicle position
    if(msg.msgid == MAVLINK_MSG_ID_GLOBAL_POSITION_INT) {
        mavlink_global_position_int_t position = {};
        mavlink_msg_global_position_int_decode(&msg, &position);

        currentPosition = coordinateSystem->computeSceneCoordinate(GeoCoord(deg(((double)position.lat)/1e7),
                deg(((double)position.lon)/1e7),
                mm(((double)position.relative_alt))));
    }

    // Update vehicle orientation
    if(msg.msgid == MAVLINK_MSG_ID_ATTITUDE) {
        mavlink_attitude_t attitude = {};
        mavlink_msg_attitude_decode(&msg, &attitude);

        currentOrientation = Quaternion(EulerAngles(rad(M_PI - attitude.yaw), rad(-attitude.roll), rad(-attitude.pitch)));
    }
}


void MAVLinkMobilityBase::receiveTelemetry(mavlink_message_t const& message) {
    Enter_Method_Silent();

    EV_DETAIL << "(" << +targetSystem << ") Received MAVLINK: " << message.msgid << std::endl;

    if(getActiveCondition() && getActiveCondition()(message) && activeInstruction != nullptr && !getActiveCompleted()) {
        activeInstruction->completed = true;
        EV_INFO << "Message " << getActiveMessage().msgid << " done." << std::endl;
    }

    nextMessageIfReady();
}

void MAVLinkMobilityBase::finish() {
    MovingMobilityBase::finish();
    cancelAndDelete(timeoutMessage);
    cancelAndDelete(heartbeatMessage);
    cancelAndDelete(updateMessage);

    if(rtScheduler != nullptr) {
        rtScheduler->removeCallback(socketFd, this);
    }

#ifdef _WIN32
    closesocket(socketFd);
#else
    close(socketFd);
#endif
    simulatorProcess->kill();

#ifdef _WIN32
    system((std::string("rd /s /q \"" ) + executionPath + "\"").c_str());
#else
    system((std::string("rm -rf " ) + executionPath).c_str());
#endif
}

MAVLinkMobilityBase::~MAVLinkMobilityBase() {
    simulatorProcess->kill();

#ifdef _WIN32
    system((std::string("rd /s /q \"" ) + executionPath + "\"").c_str());
#else
    system((std::string("rm -rf " ) + executionPath).c_str());
#endif
}

}
