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

std::size_t hash_vector(const std::vector<unsigned int>& vector) {
  std::size_t seed = vector.size();
  for(auto value : vector) {
    hashValue(value);
    incorporateHash(seed, value);
  }
  return seed;
}


void CentralizedQLearning::initialize(int stage)
{
    if(stage == 0) {
        learningRate = par("learningRate");
        gamma = par("gamma");
        epsilonDecay = par("episilon_decay");

        timeInterval =  par("timeInterval");
        trainingTimeout = par("trainingTimeout");
        distanceInterval = par("distanceInterval");

        // Epsilon starts with a value of 1 and will slowly decay during training
        epsilon = 1;
    }
    if(stage == 1) {
        populatePossibleCommands();
        scheduleAt(simTime() + timeInterval, trainingTimer);
    }
}

void CentralizedQLearning::handleMessage(cMessage *msg)
{
    if(msg->isSelfMessage()) {
        switch(msg->getKind()) {

        case TRAIN:
            trainIfReady();
            scheduleAt(simTime() + timeInterval, msg);
            break;

        case TRAIN_TIMEOUT:
            train();
            break;

        }

    } else {
        cSimpleModule::handleMessage(msg);
    }
}

void CentralizedQLearning::registerAgent(CentralizedQAgent *agent) {
    agents.push_back(agent);
}

void CentralizedQLearning::registerSensor(CentralizedQSensor *sensor) {
    sensors.push_back(sensor);
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
    GlobalState X = {};
    for(CentralizedQAgent* agent : agents) {
        X.push_back(agent->getAgentState());
    }
    /*********************************************/

    /****** Computing current optimal command ******/
    JointCommand U = {};
    int index = 0;
    // If a random variable falls under epsilon, we generate a random command
    if(uniform(0, 1) > epsilon) {
        for(CentralizedQAgent* agent : agents) {
            // Generating a valid random command
            LocalCommand command = generateRandomCommand(index);

            U.push_back(command);
            index++;
        }
    // Else we pick the command with the minimum Q-Value for the current state
    } else {
        JointCommand bestCommand = {};
        double bestQValue = std::numeric_limits<double>::infinity();
        bool commandFound = false;

        for(const JointCommand& Ucandidate : possibleCommandList) {
            QTableKey key = {X, Ucandidate};

            if(qTable.count(key) == 0) {
                continue;
            }

            double qValue = qTable[key];
            if(qValue < bestQValue) {
                bestQValue = qValue;
                bestCommand = Ucandidate;
                commandFound = true;
            }
        }
        if(!commandFound) {
            for(CentralizedQAgent* agent : agents) {
               // Generating a valid random command
               LocalCommand command = generateRandomCommand(index);

               U.push_back(command);
               index++;
           }
        }
        U = bestCommand;
        index++;
    }
    /***********************************************/

    double cost = computeCost(X);


    // Decaying the epsilon after training step
    epsilon -= epsilon * epsilonDecay;

    // Emitting current training cost for data collection
    emit(trainingCostSignal, cost);
    scheduleAt(simTime() + trainingTimeout, timeout);
}

double CentralizedQLearning::computeCost(const GlobalState& X) {
    double cost = 0;
    for(const LocalState& state : X) {
        for(CentralizedQSensor *sensor : sensors) {
            cost += sensor->getAwaitingPackages() * (*std::max_element(state.second.begin(), state.second.end()));
        }
    }
}

bool CentralizedQLearning::commandIsValid(const LocalCommand& command, unsigned int agent) {
    return command.second != agent;
}

LocalCommand CentralizedQLearning::generateRandomCommand(unsigned int agent) {
    LocalCommand command;
    do {
        command = LocalCommand(intuniform(0, 1), intuniform(0, agents.size() - 1));
    } while(commandIsValid(command, agent));
    return command;
}

void CentralizedQLearning::generateJointCommand(std::vector<LocalCommand> possibleLocalCommands, JointCommand commandChain) {
    if(commandChain.size() == agents.size()) {
        possibleCommandList.push_back(commandChain);
    }
}

void CentralizedQLearning::populatePossibleCommands() {
    std::vector<LocalCommand> possibleLocalCommands = {};
    for(unsigned char movement = 0; movement <= 1; movement++) {
        for(unsigned char sharing = 0; sharing <= agents.size(); sharing++) {
            possibleLocalCommands.push_back(LocalCommand(movement, sharing));
        }
    }

    std::deque<JointCommand> commandChains;

    for(const LocalCommand& command : possibleLocalCommands) {
        commandChains.push_back({command});
    }

    int size = 1;
    while(size < agents.size()) {
        JointCommand back = commandChains.back();
        for(const JointCommand& chain : commandChains) {
            for(const LocalCommand& command : possibleLocalCommands) {
                JointCommand newChain = JointCommand(chain);
                newChain.push_back(command);
                commandChains.push_back(newChain);
            }
            commandChains.pop_front();
            if(chain == back) break;
        }
        size++;
    }

    for(const JointCommand& command : commandChains) {
        possibleCommandList.push_back(command);
    }
}

CentralizedQLearning::~CentralizedQLearning() {
    cancelAndDelete(trainingTimer);
}

} //namespace
