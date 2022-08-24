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

#ifndef __GRADYS_SIMULATIONS_CENTRALIZEDQLEARNING_H_
#define __GRADYS_SIMULATIONS_CENTRALIZEDQLEARNING_H_

#include <omnetpp.h>
#include <vector>
#include <unordered_map>

using namespace omnetpp;

namespace projeto {


// This is the command set local an agent. It is composed of a mobility component
// and a communication component.
using LocalCommand = std::pair<unsigned char, unsigned char>;

// This is the system1s joint command, composed of all the agent's local commands.
using JointCommand = std::vector<LocalCommand>;

// This is the state local to an agent, it is composed of a localization component
// and a vector recording data stored by this agend and it's origin.
using LocalState = std::pair<int, std::vector<unsigned int>>;

// This is the system's global state, composed of all the agent's local states
using GlobalState = std::vector<LocalState>;

// This is the key used to index into the Q Table
using QTableKey = std::pair<GlobalState, JointCommand>;

/****** QTableKey hashing ******/

// https://stackoverflow.com/a/72073933
void hashValue(unsigned int &value);

// https://stackoverflow.com/a/72073933
void incorporateHash(std::size_t& hash,unsigned int value);

std::size_t hash_vector(const std::vector<unsigned int>& vector);


struct QTableKeyHash {
public:
    QTableKeyHash();
    std::size_t operator() (const QTableKey& key) const {
        std::size_t hash = key.first.size() + key.second.size();
        // https://stackoverflow.com/a/72073933
        for(const LocalState& state : key.first) {
            unsigned int value = hash_vector(state.second);
            hashValue(value);
            incorporateHash(hash, value);
        }

        for(const LocalCommand& command : key.second) {
            unsigned int value = command.first ^ command.second;
            hashValue(value);
            incorporateHash(hash, value);
        }
        return hash;
    }
};

/*******************************/

// Message types used for timing the module's execution
enum CentralizedQLearningMessages {
    TRAIN,
    TRAIN_TIMEOUT
};

class CentralizedQLearning : public cSimpleModule
{
public:
    // Abstract class that describes an agent of the Centralized Q Learning process
    class CentralizedQAgent {
    public:
        // Gets the agent's current state
        virtual const LocalState& getAgentState();

        // Applies a command to the agent
        virtual void applyCommand(const LocalCommand& command);

        // Determines if an agent has already applied the last commant it received
        virtual bool isReady();

    };

    class CentralizedQSensor {
    public:
        virtual int getAwaitingPackages();
    };

    // Registers the centralized agent "agent"
    virtual void registerAgent(CentralizedQAgent *agent);

    // Registers a centralized sensor "sensor"
    virtual void registerSensor(CentralizedQSensor *sensor);


protected:
    // OMNeT++ functions
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

    // Training functions
    virtual void trainIfReady();
    virtual void train();
    virtual double computeCost(const GlobalState& X);

    // Helpers
    virtual bool commandIsValid(const LocalCommand& command, unsigned int agent);
    virtual LocalCommand generateRandomCommand(unsigned int agent);

    // Destructor
    virtual ~CentralizedQLearning();

protected:
    // Vector of registered agents
    std::vector<CentralizedQAgent*> agents;

    // Vector of registered sensors
    std::vector<CentralizedQSensor*> sensors;

    // Messages used to time the module's execution
    cMessage *trainingTimer = new cMessage(nullptr, CentralizedQLearningMessages::TRAIN);
    cMessage *timeout = new cMessage(nullptr, CentralizedQLearningMessages::TRAIN_TIMEOUT);

    // Hyper-parameters
    double learningRate;
    double gamma;
    double epsilonDecay;

    // Simulation parameters
    double timeInterval;
    double trainingTimeout;
    double distanceInterval;

    // Training variables
    double epsilon = 1;
    std::unordered_map<std::pair<GlobalState, JointCommand>, double, QTableKeyHash> qTable = {};
    // This is an auxiliary vector that contains the optimal joint command for every global state.
    // This severely reduces the computation cost of find the JointCommand argmin for a specific state on the QTable.
    std::unordered_map<GlobalState, const JointCommand&> optimalCommandVector = {};

    // Helper variables
    // Variables for debugging and data collection
    simsignal_t trainingCostSignal = registerSignal("trainingCost");

};

} //namespace

#endif
