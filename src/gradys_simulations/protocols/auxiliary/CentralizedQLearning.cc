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

#include "CentralizedQLearning.h"
#include <algorithm>
#include <deque>
#include <unordered_set>
#include <fstream>
#include <regex>

namespace gradys_simulations {

Define_Module(CentralizedQLearning);

void hashValue(uint32_t &value) {
    value = ((value >> 16) ^ value) * 0x45d9f3b;
    value = ((value >> 16) ^ value) * 0x45d9f3b;
    value = (value >> 16) ^ value;
}

// https://stackoverflow.com/a/72073933
void incorporateHash(std::size_t& hash, uint32_t value) {
    hash ^= value + 0x9e3779b9 + (hash << 6) + (hash >> 2);
}

std::size_t hashVector(const std::vector<uint32_t>& vector) {
  std::size_t seed = vector.size();
  for(auto value : vector) {
    hashValue(value);
    incorporateHash(seed, value);
  }
  return seed;
}

std::size_t GlobalStateHash::operator()(const GlobalState& key) const {
   std::size_t hash = key.agents.size();
   for(const LocalState& state : key.agents) {
       uint32_t value = state.communication;
       hashValue(value);
       incorporateHash(hash, value);

       value = state.mobility;
       hashValue(value);
       incorporateHash(hash, value);
   }
   incorporateHash(hash, hashVector(key.sensors));
   return hash;
}

std::size_t QTableKeyHash::operator()(const QTableKey& key) const {
   std::size_t hash = key.first.agents.size() + key.second.size();
   // https://stackoverflow.com/a/72073933
   incorporateHash(hash, static_cast<uint32_t>(GlobalStateHash()(key.first)));

   for(const LocalControl& command : key.second) {
       uint32_t value = command.mobility;
       hashValue(value);
       incorporateHash(hash, value);
   }
   return hash;
}

void CentralizedQLearning::initialize(int stage)
{
    if(stage == 0) {
        trainingMode = par("trainingMode");
        qTablePath = strdup(par("qTablePath").stringValue());

        learningRate = par("learningRate");
        gamma = par("gamma");
        epsilonDecayStrategy = static_cast<EpsilonDecayStrategy>(par("epsilonDecayStrategy").intValue());
        epsilonHorizon = static_cast<double>(par("epsilonHorizon").intValue());
        epsilonSteps = par("epsilonSteps");
        epsilonStart = par("epsilonStart");
        epsilonEnd = par("epsilonEnd");
        epsilonShortCircuit = par("epsilonShortCircuit");
        startFromScratch = par("startFromScratch");

        sensorStorageTolerance = par("sensorStorageTolerance");

        timeInterval =  par("timeInterval");
        distanceInterval = par("distanceInterval");

        communicationStorageInterval = par("communicationStorageInterval");

        maxDiscreteAgentPackets = par("maxDiscreteAgentPackets");
        maxDiscreteAwaitingPackets = par("maxDiscreteAwaitingPackets");

        costFunction = par("costFunction");

        agentWeight = par("agentWeight");
        sensorWeight = par("sensorWeight");
        throughputWeight = par("throughputWeight");



        // Epsilon starts with a value of 1 and will slowly decay during training
        epsilon = epsilonStart;

        WATCH(epsilon);
        WATCH(trainingSteps);
        WATCH(lastCost);
    }
    if(stage == 1) {
        initializeQTable();
        scheduleAt(simTime() + timeInterval, trainingTimer);
    }
}

void CentralizedQLearning::handleMessage(cMessage *msg)
{
    if(msg->isSelfMessage()) {
        if(msg == trainingTimer) {
            trainIfReady();
            scheduleAt(simTime() + timeInterval, msg);
        }
    } else {
        cSimpleModule::handleMessage(msg);
    }
}

CentralizedQLearning::AgentInfo CentralizedQLearning::registerAgent(CentralizedQAgent *agent) {
    agents.push_back(agent);
    AgentInfo info = {static_cast<int>(agents.size() - 1), distanceInterval, communicationStorageInterval};
    return info;
}

int CentralizedQLearning::registerSensor(CentralizedQSensor *sensor) {
    sensors.push_back(sensor);
    return sensors.size() - 1;
}

void CentralizedQLearning::registerGround(CentralizedQGround *groundStation) {
    ground = groundStation;
}

void CentralizedQLearning::trainIfReady() {
    bool allReady = true;
    for(CentralizedQAgent* agent : agents) {
        bool agentReady = agent->isReady();
        allReady = agentReady;
        if(!agentReady) {
            break;
        }
    }

    if(allReady) {
        train();
    }
}

void CentralizedQLearning::train() {
    /****** Collecting current global state ******/
    GlobalState newState = {};

    for(CentralizedQAgent* agent : agents) {
        uint32_t packets = agent->getCollectedPackets();
        packets = std::floor(packets / communicationStorageInterval);
        if (packets > maxDiscreteAgentPackets) {
            packets = maxDiscreteAgentPackets;
        }

        double position = agent->getCurrentPosition();
        position = std::floor(position / distanceInterval);

        LocalState state = {
                static_cast<uint16_t>(position),
                packets
        };
        newState.agents.push_back(state);
    }

    for(auto sensor : sensors) {
        auto value = std::floor(static_cast<double>(sensor->getAwaitingPackets()) / sensorStorageTolerance);
        if(value > maxDiscreteAwaitingPackets) {
            value = maxDiscreteAwaitingPackets;
        }
        newState.sensors.push_back(value);
    }

    /*********************************************/

    if(trainingMode) {
        if(trainState == DECISION) {
            X = newState;
            /****** Computing current optimal (or random) command ******/
            // If a random variable falls under epsilon, we generate a random command
            if(uniform(0, 1) < epsilon) {
                U = generateRandomJointControl();
            // Else we pick the command with the minimum Q-Value for the current state
            } else {
                JointControl optimalControl = {};

                if(optimalControlMap.count(X) == 0) {
                    U = generateRandomJointControl();
                } else {
                    U = optimalControlMap[X];
                }
            }
            /***********************************************/


            dispatchJointCommand();
            trainState = LEARNING;
        } else {
            // Computing cost
            double cost = computeCost(newState);
            lastCost = cost;
            // Emitting current training cost for data collection
            emit(trainingCostSignal, cost);

            /****** Updating Q Table *****/
            // Key that will be updated
            QTableKey key = {X, U};
            // Previous value held by that key
            double previousQValue = (QTable.count(key) == 0) ? 0 : QTable[key];

            // TODO: Try using min instead of optimalControlMap

            // Optimal control for the next state
            // If the optimal control map doesn't have a optimal control stored for this new state, it means that the QTable row for this state
            // has no values. In that case we generate a random joint control.
            JointControl nextOptimalControl = (optimalControlMap.count(newState) == 0) ? generateRandomJointControl() : optimalControlMap[newState];

            // Getting the Q Value for the next stage
            QTableKey nextKey = {newState, nextOptimalControl};
            double nextStateQValue = (QTable.count(nextKey) == 0) ? 0 : QTable[nextKey];

            // Calculating the new QValue and updating the QTable
            double QValue = (1 - learningRate) * previousQValue + learningRate * (lastCost + gamma * nextStateQValue);
            QTable[key] = QValue;

            // Updating the optimal control map
            if(optimalControlMap.count(X) == 0 || QValue < QTable[{X, optimalControlMap[X]}]) {
                optimalControlMap[X] = U;
            }
            /*****************************/

            decayEpsilon();

            trainState = DECISION;
            trainingSteps++;

            // Immediately start DECISION step
            train();
        }
    } else {
        double cost = computeCost(newState);
        lastCost = cost;
        // Emitting current training cost for data collection
        emit(trainingCostSignal, cost);

        X = newState;
        // If the module is in testing mode, use the optimal control map to get the optimal command
        JointControl optimalControl = {};

        if(optimalControlMap.count(X) == 0) {
            U = generateRandomJointControl();
        } else {
            U = optimalControlMap[X];
        }
        /***********************************************/

        dispatchJointCommand();
        trainingSteps++;
    }
}

void CentralizedQLearning::dispatchJointCommand() {
    int index = 0;
    for(CentralizedQAgent* agent : agents) {
        agent->applyCommand(U[index]);
        index++;
    }
}

double CentralizedQLearning::computeCost(const GlobalState& newState) {
    if (costFunction == 1) {
        // Weighed average cost
        double maximumDistance = 0;
        if (agents.size() > 0) {
            maximumDistance = std::floor(agents[0]->getMaximumPosition() / distanceInterval);
        }

        double agentCost = 0;
        int index = 0;
        for (auto state: newState.agents) {
            agentCost += (agents[index]->getCollectedPackets()) * (state.mobility / maximumDistance);
            index++;
        }

        agentCost /= agents.size();

        double sensorCost = 0;
        index = 0;
        for(auto value: newState.sensors) {
            sensorCost += (sensors[index]->hasBeenVisited() ? sensors[index]->getAwaitingPackets() : maxDiscreteAwaitingPackets);
            index++;
        }
        sensorCost /= sensors.size();

        double throughput =  ground->getReceivedPackets() / simTime();

        if (agentWeight + sensorWeight + throughputWeight == 0) {
            return 0;
        }

        double cost = (agentCost * agentWeight + sensorCost * sensorWeight + (1 - throughput) * throughputWeight) / (agentWeight + sensorWeight + throughputWeight);

        return cost;
    } else if (costFunction == 2) {
        // Average packet position
        double maximumDistance = 0;
        if (agents.size() > 0) {
            maximumDistance = std::floor(agents[0]->getMaximumPosition() / distanceInterval);
        }

        double cost = 0;
        double packetCount = 0;
        unsigned int index = 0;
        for(auto state: newState.agents) {
            cost += agents[index]->getCollectedPackets() * (state.mobility / maximumDistance) * agentWeight;
            packetCount += agents[index]->getCollectedPackets();
            index++;
        }

        for(auto sensor: sensors) {
            cost += sensor->getAwaitingPackets() * sensor->getSensorPosition() * sensorWeight;
            packetCount += sensor->getAwaitingPackets();
        }

        packetCount += ground->getReceivedPackets();

        if (packetCount == 0) {
            cost = 0;
        } else {
            cost /= packetCount;
        }

        return cost;
    } else if (costFunction == 3) {
        // Max packet position
        double maximumDistance = 0;
        if (agents.size() > 0) {
            maximumDistance = agents[0]->getMaximumPosition();
        }

        double maxPosition = 0;

        for(auto agent: agents) {
            auto collectedPackets = agent->getCollectedPackets();
            if (collectedPackets > communicationStorageInterval && (agent->getCurrentPosition() / maximumDistance) > maxPosition) {
                maxPosition = (agent->getCurrentPosition() / maximumDistance);
            }
        }

        for(auto sensor: sensors) {
            auto awaitingPackets = sensor->getAwaitingPackets();
            if (awaitingPackets > sensorStorageTolerance && sensor->getSensorPosition() > maxPosition) {
                maxPosition = sensor->getSensorPosition();
            }
        }

        return maxPosition;
    } else if (costFunction == 4) {
        // Packet count in system
        double packetCount = 0;
        for(auto agent: agents) {
            packetCount += agent->getCollectedPackets() / (agent->getMaxCollectedPackets() / 3);
        }

        for(auto sensor: sensors) {
            packetCount += sensor->getAwaitingPackets() / sensor->getMaxAwaitingPackets();
        }
        packetCount /= sensors.size();

        return packetCount;
    }
    return 0;
}

void CentralizedQLearning::decayEpsilon() {
    if (trainingSteps > epsilonHorizon) {
        // If the epsilon short circuit is on the simulation ends
        // after the horizon is reached.
        if (epsilonShortCircuit) {
            endSimulation();
        }

        epsilon = epsilonEnd;
        return;
    }
    switch(epsilonDecayStrategy) {
    case LINEAR:
        epsilon -= ((epsilonStart - epsilonEnd) / epsilonHorizon);
        break;
    case EXPONENTIAL:
        epsilon *= std::pow((epsilonEnd/epsilonStart), 1/epsilonHorizon);
        break;
    case STEPS:
    {
        if(trainingSteps % static_cast<int>(std::floor(epsilonHorizon / epsilonSteps)) == 0 && trainingSteps > 0)
        {
            epsilon -= (epsilonStart - epsilonEnd) / epsilonSteps;
        }
        break;
    }
    }
    emit(epsilonSignal, epsilon);
}

void CentralizedQLearning::initializeQTable() {
    QTable = {};
    if (!startFromScratch) {
        importQTable();
    }
}

void CentralizedQLearning::exportQTable() {
    std::ofstream file;
    file.open(qTablePath, std::ofstream::out | std::ofstream::trunc);

    if(!file.is_open()) {
        EV_ERROR << "Error opening output file " << qTablePath << " for the Q Table: " << strerror(errno) << " (" << errno << ")" << std::endl;
        return;
    }

    file << "[\n";

    std::unordered_set<GlobalState, GlobalStateHash> visitedStates = {};
    int tableIndex = 0;
    for(auto iter: QTable) {
        const QTableKey& key = iter.first;
        const GlobalState& globalState = key.first;
        const JointControl& jointControl = optimalControlMap[globalState];

        // Preventing repeated states from being visited
        if(visitedStates.count(globalState) == 0) {
            visitedStates.insert(globalState);
        } else {
            continue;
        }

        file << "  {\n";
        file << "    \"globalState\": {\n";
        file << "      \"mobility\": [";


        for(int index = 0; index < globalState.agents.size(); index++) {
            file << globalState.agents[index].mobility;
            if(index < globalState.agents.size() - 1) {
                file << ", ";
            }
        }
        file << "],\n";
        file << "      \"communication\": [";

        for(int index = 0; index < globalState.agents.size(); index++) {
            file << globalState.agents[index].communication;
            if(index < globalState.agents.size() - 1) {
                file << ", ";
            }
        }

        file << "],\n";

        file << "      \"sensors\": [";

        for(int index = 0; index < globalState.sensors.size(); index++) {
            file << globalState.sensors[index];
            if(index < globalState.sensors.size() - 1) {
                file << ", ";
            }
        }

        file << "]\n";


        file << "    },\n";
        file << "    \"jointControl\": [";

        for(int index = 0; index < jointControl.size(); index++) {
            file << +jointControl[index].mobility;
            if(index < jointControl.size() - 1) {
                file << ", ";
            }
        }
        file << "],\n";
        file << "    \"qValue\": " << iter.second << "\n";

        file << "  }";
        if (tableIndex < QTable.size() - 1) {
            file << ",";
        }
        file << "\n";
        tableIndex++;
    }
    file << "]";

    file.close();
}

std::vector<uint32_t> parseVectorString(std::string str) {
    str.erase(std::remove(str.begin(), str.end(), '['), str.end());
    str.erase(std::remove(str.begin(), str.end(), ']'), str.end());

    size_t pos = 0;
    std::vector<uint32_t> vector;
    std::string delimiter = ", ";
    while ((pos = str.find(delimiter)) != std::string::npos) {
        std::string token = str.substr(0, pos);
        vector.push_back(atoi(token.c_str()));
        str.erase(0, pos + delimiter.length());
    }
    // Adding last element remaining after delimiter
    if(str.size() > 0) {
        vector.push_back(atoi(str.c_str()));
    }


    return vector;
}

std::vector<std::vector<uint32_t>> parseNestedVectorString(std::string str) {
    std::vector<std::vector<uint32_t>> nestedVector;

    std::regex arrayMatcher("\\[[^[\\]]*\\]");

    auto words_begin = std::sregex_iterator(str.begin(), str.end(), arrayMatcher);
    auto words_end = std::sregex_iterator();

    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        nestedVector.push_back(parseVectorString(match.str()));
    }

    return nestedVector;
}

void CentralizedQLearning::importQTable() {
    std::ifstream file;
    file.open(qTablePath);

    std::regex tableMatcher(
        "\\{\\s*\"globalState\":\\s*\\{\\s*\"mobility\":\\s*(\\[.*\\]),\\s*\"communication\":\\s*(\\[.*\\]),\\s*\"sensors\":\\s*(\\[.*\\])\\s*\\},\\s*\"jointControl\":\\s*(\\[.*\\]),\\s*\"qValue\":\\s*(.*)\\s*\\},?"
    );

    if(!file.good()) {
        EV_ERROR << "Error opening input file " << qTablePath << " for the Q Table: " << strerror(errno) << " (" << errno << ")" << std::endl;
        return;
    }

    optimalControlMap = {};

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string fileText = buffer.str();
    auto words_begin =
            std::sregex_iterator(fileText.begin(), fileText.end(), tableMatcher);
    auto words_end = std::sregex_iterator();

    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        std::vector<uint32_t> mobilityStates = parseVectorString(match[1].str());
        std::vector<uint32_t> communicationStates = parseVectorString(match[2].str());
        std::vector<uint32_t> sensorPackets = parseVectorString(match[3].str());
        std::vector<uint32_t> jointControlVector = parseVectorString(match[4].str());
        double qValue = std::atof(match[5].str().c_str());

        size_t stateSize = mobilityStates.size();
        if(communicationStates.size() != stateSize || jointControlVector.size() != stateSize) {
            EV_ERROR << "Error reading key-value object at index " << std::distance(words_begin, i) << std::endl;
        }

        GlobalState globalState = {};
        JointControl jointControl = {};
        for(size_t index = 0; index < stateSize; index++) {
            LocalState state = {};
            LocalControl control = {};
            state.mobility = mobilityStates[index];
            state.communication = communicationStates[index];

            control.mobility = jointControlVector[index];

            globalState.agents.push_back(state);
            jointControl.push_back(control);
        }
        globalState.sensors = sensorPackets;
        optimalControlMap[globalState] = jointControl;
        QTable[{globalState, jointControl}] = qValue;
    }
}


LocalControl CentralizedQLearning::generateRandomLocalControl() {
    return {static_cast<uint8_t>(intuniform(0, 1))};
}

JointControl CentralizedQLearning::generateRandomJointControl() {
    JointControl U = {};
    for(int index = 0;index < agents.size(); index++) {
       // Generating a valid random command
       LocalControl command = generateRandomLocalControl();

       U.push_back(command);
   }
    return U;
}

void CentralizedQLearning::finish() {
    cancelAndDelete(trainingTimer);

    // Exporting QTable if we were training
    if (trainingMode) {
        exportQTable();
    }
}

} //namespace
