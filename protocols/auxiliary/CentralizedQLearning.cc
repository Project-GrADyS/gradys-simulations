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

namespace projeto {

Define_Module(CentralizedQLearning);

void hashValue(unsigned int &value) {
    value = ((value >> 16) ^ value) * 0x45d9f3b;
    value = ((value >> 16) ^ value) * 0x45d9f3b;
    value = (value >> 16) ^ value;
}

// https://stackoverflow.com/a/72073933
void incorporateHash(std::size_t& hash,unsigned int value) {
    hash ^= value + 0x9e3779b9 + (hash << 6) + (hash >> 2);
}

std::size_t hashVector(const std::vector<unsigned int>& vector) {
  std::size_t seed = vector.size();
  for(auto value : vector) {
    hashValue(value);
    incorporateHash(seed, value);
  }
  return seed;
}

std::size_t GlobalStateHash::operator()(const GlobalState& key) const {
   std::size_t hash = key.size();
   for(const LocalState& state : key) {
       unsigned int value = hashVector(state.second) ^ static_cast<std::size_t>(state.first);
       hashValue(value);
       incorporateHash(hash, value);
   }
   return hash;
}

std::size_t QTableKeyHash::operator()(const QTableKey& key) const {
   std::size_t hash = key.first.size() + key.second.size();
   // https://stackoverflow.com/a/72073933
   for(const LocalState& state : key.first) {
       unsigned int value = hashVector(state.second);
       hashValue(value);
       incorporateHash(hash, value);
   }

   for(const LocalControl& command : key.second) {
       unsigned int value = command.first ^ command.second;
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

        costFunction = static_cast<CostFunction>(par("costFunction").intValue());

        timeInterval =  par("timeInterval");
        distanceInterval = par("distanceInterval");

        communicationStorageInterval = par("communicationStorageInterval");

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
        LocalState state = agent->getAgentState();


        // TODO: Remove
        if(costFunction == SANITY || costFunction == SANITY_2) {
            //Zeroing out every communication state
            for(int i=0;i<state.second.size();i++) {
                state.second[i] = 0;
            }
        }
        newState.push_back(state);
    }
    if(costFunction == SANITY_2) {
        for(int i = 0; i < sensors.size(); i++) {
            unsigned int awaiting = sensors[i]->getAwaitingPackets();
            if (awaiting > 80) {
                newState[0].second[i] = 2;
            } else if (awaiting > 10) {
                newState[0].second[i] = 1;
            } else {
                newState[0].second[i] = 0;
            }
        }
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
    switch(costFunction) {
    case DEFAULT:
    {
        double cost = 0;
        for(CentralizedQAgent *agent : agents) {
            std::vector<unsigned int> agentPackets = agent->getCollectedPackets();

            unsigned int maxPacket = 0;
            for(auto packet : agentPackets) {
                if (packet > maxPacket) {
                    maxPacket = packet;
                }
            }

            for(CentralizedQSensor *sensor : sensors) {
                cost += sensor->getAwaitingPackets() * maxPacket;
            }
        }

        cost /= agents.size() * sensors.size();

        return cost;
    }
    case SIMPLIFIED:
    {
        double cost = 0;
        for(CentralizedQSensor *sensor : sensors) {
            cost += sensor->getAwaitingPackets();
        }

        cost /= sensors.size();

        return cost;
    }

    // Large cost for moving away from ground station while carrying packets
    // Cost for sharing while it has no packets - False alarm cost -- OMMITING THIS ONE FOR NOW
    // Large cost for sending data further away from the ground station
    case CONDITIONAL:
    {
        double cost = 0;
        for(CentralizedQSensor *sensor : sensors) {
            cost += sensor->getAwaitingPackets();
        }

        for(int index = 0; index < agents.size(); index++) {
            std::vector<unsigned int> packets = X[index].second;
            // Checking if the agent has packets
            bool hasPackets = false;
            for(unsigned int load: packets) {
                if (load > 0) {
                    hasPackets = true;
                    break;
                }
            }

            // Checks if the agent is moving away from the ground station and if it previously had packets
            if(hasPackets && U[index].first == 0) {
                cost += 100; // Punishes if agent is moving away from ground station with packets
            }


            // If the target is further away and the agent sent it's packets to it,
            unsigned char target = U[index].second;
            if(!hasPackets && X[target].first > X[index].first) {
                cost += 100; // Punish for sending data away from the ground station
            }
        }

        return cost;
    }

    // Notes 2:
    // Negative throughput
    case THROUGHPUT:
    {
        return -(ground->getReceivedPackets()/simTime());
    }
    case SANITY:
    {
        double distanceSum = 0;
        for(const LocalState& state : newState) {
            distanceSum += state.first;
        }
        return distanceSum / agents.size();
    }
    case SANITY_2:
    {
        unsigned int sum = 0;
        for(auto sensor: sensors) {
            sum += sensor->getAwaitingPackets();
        }
        return sum;
    }
    }
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
    if(trainingMode) {
        QTable = {};
    } else {
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


        for(int index = 0; index < globalState.size(); index++) {
            file << globalState[index].first;
            if(index < globalState.size() - 1) {
                file << ", ";
            }
        }
        file << "],\n";
        file << "      \"communication\": [";

        for(int index = 0; index < globalState.size(); index++) {
            file << "[";
            for(int commIndex = 0; commIndex < globalState[index].second.size(); commIndex++) {
                file << globalState[index].second[commIndex];
                if(commIndex < globalState[index].second.size() - 1) {
                    file << ", ";
                }
            }
            file << "]";
            if(index < globalState.size() - 1) {
                file << ", ";
            }
        }

        file << "]\n";
        file << "    },\n";
        file << "    \"jointControl\": [";

        for(int index = 0; index < jointControl.size(); index++) {
            file << "[" << +jointControl[index].first << ", " << +jointControl[index].second << "]";
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

std::vector<unsigned int> parseVectorString(std::string str) {
    str.erase(std::remove(str.begin(), str.end(), '['), str.end());
    str.erase(std::remove(str.begin(), str.end(), ']'), str.end());

    size_t pos = 0;
    std::vector<unsigned int> vector;
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

std::vector<std::vector<unsigned int>> parseNestedVectorString(std::string str) {
    std::vector<std::vector<unsigned int>> nestedVector;

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
        "\\{\\s*\"globalState\":\\s*\\{\\s*\"mobility\":\\s*(\\[.*\\]),\\s*\"communication\":\\s*(\\[.*\\])\\s*\\},\\s*\"jointControl\":\\s*(\\[.*\\]),\\s*\"qValue\":\\s*(.*)\\s*\\},?"
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
        std::vector<unsigned int> mobilityStates = parseVectorString(match[1].str());
        std::vector<std::vector<unsigned int>> communicationStates = parseNestedVectorString(match[2].str());
        std::vector<std::vector<unsigned int>> jointControlVector = parseNestedVectorString(match[3].str());

        size_t stateSize = mobilityStates.size();
        if(communicationStates.size() != stateSize || jointControlVector.size() != stateSize) {
            EV_ERROR << "Error reading key-value object at index " << std::distance(words_begin, i) << std::endl;
        }

        GlobalState globalState = {};
        JointControl jointControl = {};
        for(size_t index = 0; index < stateSize; index++) {
            LocalState state = {};
            LocalControl control = {};
            state.first = mobilityStates[index];
            state.second = communicationStates[index];

            control.first = jointControlVector[index][0];
            control.second = jointControlVector[index][1];

            globalState.push_back(state);
            jointControl.push_back(control);
        }
        optimalControlMap[globalState] = jointControl;
    }
}


LocalControl CentralizedQLearning::generateRandomLocalControl(unsigned int agent) {
    return LocalControl(intuniform(0, 1), intuniform(0, agents.size() - 1));
}

JointControl CentralizedQLearning::generateRandomJointControl() {
    JointControl U = {};
    for(int index = 0;index < agents.size(); index++) {
       // Generating a valid random command
       LocalControl command = generateRandomLocalControl(index);
       if (costFunction == SANITY_2) {
           command.second = 0;
       }

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
