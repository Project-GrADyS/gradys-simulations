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
        learningRate = par("learningRate");
        gamma = par("gamma");
        epsilonDecay = par("epsilonDecay");

        timeInterval =  par("timeInterval");
        trainingTimeout = par("trainingTimeout");
        distanceInterval = par("distanceInterval");

        // Epsilon starts with a value of 1 and will slowly decay during training
        epsilon = 1;

        WATCH(epsilon);
        WATCH(trainingSteps);
        WATCH(lastCost);
    }
    if(stage == 1) {
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
    AgentInfo info = {static_cast<int>(agents.size() - 1), distanceInterval};
    return info;
}

int CentralizedQLearning::registerSensor(CentralizedQSensor *sensor) {
    sensors.push_back(sensor);
    return sensors.size() - 1;
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

        // Rounding location to the first increment of distance interval
        state.first = distanceInterval * std::round(state.first / distanceInterval);
        newState.push_back(state);
    }
    /*********************************************/

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
            }
            U = optimalControlMap[X];
        }
        /***********************************************/

        dispatchJointCommand();
        trainState = LEARNING;
    } else {
        // Computing cost
        double cost = computeCost(X);
        lastCost = cost;

        // Emitting current training cost for data collection
        emit(trainingCostSignal, cost);

        /****** Updating Q Table *****/
        // Key that will be updated
        QTableKey key = {X, U};
        // Previous value held by that key
        double previousQValue = (QTable.count(key) == 0) ? 0 : QTable[key];

        // Optimal control for the next state
        // If the optimal control map doesn't have a optimal control stored for this new state, it means that the QTable row for this state
        // has no values. In that case we generate a random joint control.
        JointControl nextOptimalControl = (optimalControlMap.count(newState) == 0) ? generateRandomJointControl() : optimalControlMap[newState];

        // Getting the Q Value for the next stage
        QTableKey nextKey = {newState, nextOptimalControl};
        double nextStateQValue = (QTable.count(nextKey) == 0) ? 0 : QTable[nextKey];

        // Calculating the new QValue and updating the QTable
        double QValue = (1 - learningRate) * previousQValue + learningRate * (cost + gamma * nextStateQValue);
        QTable[key] = QValue;

        // Updating the optimal control map
        if(optimalControlMap.count(X) == 0 || QValue < QTable[{X, optimalControlMap[X]}]) {
            optimalControlMap[X] = U;
        }
        /*****************************/

        // Decaying the epsilon after training step
        epsilon -= epsilon * epsilonDecay;

        trainState = DECISION;
        trainingSteps++;
    }
    if(timeout->isScheduled()) {
        cancelEvent(timeout);
    }
    scheduleAt(simTime() + trainingTimeout, timeout);
}

void CentralizedQLearning::dispatchJointCommand() {
    int index = 0;
    for(CentralizedQAgent* agent : agents) {
        agent->applyCommand(U[index]);
        index++;
    }
}

double CentralizedQLearning::computeCost(const GlobalState& X) {
    double cost = 0;
    for(const LocalState& state : X) {
        for(CentralizedQSensor *sensor : sensors) {
            cost += sensor->getAwaitingPackets() + (*std::max_element(state.second.begin(), state.second.end()));
        }
    }
    return cost;
}

bool CentralizedQLearning::commandIsValid(const LocalControl& command, unsigned int agent) {
    return command.second != agent;
}

LocalControl CentralizedQLearning::generateRandomLocalControl(unsigned int agent) {
    LocalControl command;
    do {
        command = LocalControl(intuniform(0, 1), intuniform(0, agents.size() - 1));
    } while(!commandIsValid(command, agent));
    return command;
}

JointControl CentralizedQLearning::generateRandomJointControl() {
    JointControl U = {};
    int index = 0;
    for(CentralizedQAgent* agent : agents) {
       // Generating a valid random command
       LocalControl command = generateRandomLocalControl(index);

       U.push_back(command);
       index++;
   }
    return U;
}


CentralizedQLearning::~CentralizedQLearning() {
    cancelAndDelete(trainingTimer);
}

} //namespace
