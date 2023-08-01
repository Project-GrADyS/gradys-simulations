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

// This is the command set local an agent. It is composed of a mobility component
// and a communication component.
struct LocalControl {
    uint8_t mobility;

    bool operator==(const LocalControl& other) const {
        return this->mobility == other.mobility;
    }
};

// This is the system1s joint command, composed of all the agent's local commands.
using JointControl = std::vector<LocalControl>;

// This is the state local to an agent, it is composed of a localization component
// and a vector recording data stored by this agend and it's origin.
struct LocalState {
    uint16_t mobility;
    uint32_t communication;

    bool operator==(const LocalState& other) const {
        return this->mobility == other.mobility && this->communication == other.communication;
    }
};

// This is the system's global state, composed of all the agent's local states
struct GlobalState {
    std::vector<LocalState> agents;
    std::vector<uint32_t> sensors;

    bool operator==(const GlobalState& other) const {
        return this->agents == other.agents && this->sensors == other.sensors;
    }
};

// This is the key used to index into the Q Table
using QTableKey = std::pair<GlobalState, JointControl>;

namespace gradys_simulations {

/****** QTableKey hashing ******/

// https://stackoverflow.com/a/72073933
extern void hashValue(uint32_t &value);

// https://stackoverflow.com/a/72073933
extern void incorporateHash(std::size_t& hash,uint32_t value);

extern std::size_t hashVector(const std::vector<uint32_t>& vector);

class GlobalStateHash {
public:
    virtual std::size_t operator() (const GlobalState& key) const;
};

class QTableKeyHash {
public:
    virtual std::size_t operator() (const QTableKey& key) const;
};
/*******************************/

enum CentralizedQLearningState {
    DECISION,
    LEARNING
};

enum EpsilonDecayStrategy {
    LINEAR = 1,
    EXPONENTIAL = 2,
    STEPS = 3
};

class CentralizedQLearning : public cSimpleModule
{
public:
    // Abstract class that describes an agent of the Centralized Q Learning process
    class CentralizedQAgent {
    public:
        // Gets the agent's current state
        virtual double getCurrentPosition() = 0;
        virtual double getMaximumPosition() = 0;

        virtual uint32_t getCollectedPackets() = 0;

        virtual uint32_t getMaxCollectedPackets() = 0;


        // Applies a command to the agent
        virtual void applyCommand(const LocalControl& command) = 0;

        // Determines if an agent has already applied the last commant it received
        virtual bool isReady() = 0;

    };


    class CentralizedQSensor {
    public:
        virtual uint32_t getAwaitingPackets() = 0;

        virtual uint32_t getMaxAwaitingPackets() = 0;

        virtual bool hasBeenVisited() = 0;

        virtual double getSensorPosition() = 0;
    };

    class CentralizedQGround {
    public:
        virtual uint32_t getReceivedPackets() = 0;
    };


public:

    struct AgentInfo {
        int agentId;
        double distanceInterval;
        double communicationStorageInterval;
    };

    // Registers the centralized agent "agent"
    virtual AgentInfo registerAgent(CentralizedQAgent *agent);
    int agentCount() { return agents.size(); }

    // Registers a centralized sensor "sensor"
    virtual int registerSensor(CentralizedQSensor *sensor);
    int sensorCount() { return sensors.size(); }

    // Registers the ground station
    virtual void registerGround(CentralizedQGround *ground);


protected:
    // OMNeT++ functions
    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return 2; };
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;

    // Training functions
    virtual void trainIfReady();
    virtual void train();
    virtual void dispatchJointCommand();
    virtual double computeCost(const GlobalState& newState);
    virtual void decayEpsilon();

    // Helpers
    virtual void initializeQTable();
    virtual void exportQTable();
    virtual void importQTable();
    virtual LocalControl generateRandomLocalControl();
    virtual JointControl generateRandomJointControl();

protected:
    bool trainingMode;
    char* qTablePath;

    // Vector of registered agents
    std::vector<CentralizedQAgent*> agents;

    // Vector of registered sensors
    std::vector<CentralizedQSensor*> sensors;

    CentralizedQGround* ground;

    // Messages used to time the module's execution
    cMessage *trainingTimer = new cMessage(nullptr);

    // Hyper-parameters
    double learningRate;
    double gamma;
    EpsilonDecayStrategy epsilonDecayStrategy;
    double epsilonHorizon;
    long epsilonSteps;
    double epsilonStart;
    double epsilonEnd;
    bool epsilonShortCircuit;
    bool startFromScratch;

    // Simulation parameters
    double timeInterval;
    double distanceInterval;
    double communicationStorageInterval;
    double sensorStorageTolerance;
    double maxDiscreteAgentPackets;
    double maxDiscreteAwaitingPackets;
    double costFunction;

    double agentWeight;
    double sensorWeight;
    double throughputWeight;

    // Training variables
    // Current state of the traninig process. The traninign process has two states:
    // 1- DECISION: For the current state an optimal (or random) joint command will be calculated and dispatched to the agents
    // 2- LEARNING: The results from the decision will be observed in the state and an optimization step will take place
    CentralizedQLearningState trainState = DECISION;
    // Current global state
    GlobalState X = {};
    // Current joint command
    JointControl U = {};
    double epsilon = 1;
    std::unordered_map<std::pair<GlobalState, JointControl>, double, QTableKeyHash> QTable = {};
    // This is an auxiliary vector that contains the optimal joint command for every global state.
    // This severely reduces the computation cost of find the JointCommand argmin for a specific state on the QTable.
    std::unordered_map<GlobalState, JointControl, GlobalStateHash> optimalControlMap = {};

    int trainingSteps = 0;
    double lastCost = 0;

    // Helper variables
    // Variables for debugging and data collection
    simsignal_t trainingCostSignal = registerSignal("trainingCost");
    simsignal_t epsilonSignal = registerSignal("epsilon");

};

} //namespace

#endif
