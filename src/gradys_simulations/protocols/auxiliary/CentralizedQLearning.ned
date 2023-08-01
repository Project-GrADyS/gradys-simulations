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

package gradys_simulations.protocols.auxiliary;

simple CentralizedQLearning
{
    parameters:
        @class(CentralizedQLearning);
        @signal[trainingCost](type=double);
        @signal[epsilon](type=double);
        @statistic[epsilon](record=vector);
        @statistic[trainingCost](record=vector);
        @statistic[cumTrainCost](source=sum(trainingCost); record=vector);
        @statistic[cumAvgTrainCost](source=avg(trainingCost); record=vector);
        
        // Flag that dictates if the module is run in training or testing mode.
        // If set to true the module will use the hyper parameters below and train using
        // a centralized Q learning algorithm. The output of this training is a Q Table
        // exported after the simulation's end. Otherwise the module will import the Q Table
        // and use it to make decisions.
        bool trainingMode = default(true);
        
        // Path to the Centralized Q Table. If "trainingMode" is set to true the table is exported
        // to this path, otherwise it is imported from this path.
        string qTablePath;
        
        // HyperParameters for the Centralized Q-Learning algorithm
        double learningRate = default(0.1);
        double gamma = default(0.99);
        // Defines the strategy used to decay epsilon. A decay stategy defines how the epsilon value will decay from 1 to 0. 
        // The possible strategies are:
        // 1 = "linear" - In epsilonHorizon training steps epsilon will decay linearly from epsilonStart to epsilonEnd
        // 2 = "exponetial" - In episilonHorizon training steps epsilon will decay exponentially from epsilonStart to epsilonEnd
        // 3 = "steps" - In episilonHorizon training steps the episilon will decay by ((epsilonEnd - epsilonStart) / episilonSteps) every episilonSteps training steps
        int epsilonDecayStrategy = default(2);
        double epsilonStart = default(1);
        double epsilonEnd = default(0.001);
        int epsilonHorizon = default(100000);
        // Only relevant to the "steps" strategy
        int epsilonSteps = default(10);
        
        // If this boolean is set to true the simulation will automatically end when
        // the epsilon horizon is reached
        bool epsilonShortCircuit = default(false);
        
        // Determines if this simulation should start from scratch, ignoring any previous QTables, or if it
        // should reuse and add knowledge to one, if available
        bool startFromScratch = default(true);

        // Parameters for the simulated scenario
        double timeInterval @unit(s) = default(100ms); // For our MAMDP model time has to be discrete. This parameter specifies the interval between 
                                                       // each discretized time state. Agents may not be ready for another step after at this interval.

        double distanceInterval @unit(m) = default(80m); // For our MAMDP model the location component of the state space has to be discrete. This
        											    // parameter specifies the interval between each discretized location stage.
        											    
     	double communicationStorageInterval = default(80); // This parameters allows the simplification of the communication part of the local state.
     													   // The values stored in each of the indexes of the agent's local state are going to be divided
     													   // by communicationStorageInterval and rounded up.
     	double sensorStorageTolerance = default(80); // Awaiting packets below this value will be considered as no awaiting packets at all
     	
     	double maxDiscreteAgentPackets = default(3);
     	double maxDiscreteAwaitingPackets = default(3);
     	
     	double costFunction = default(1);
     	
     	double agentWeight = default(1);
        double sensorWeight = default(1);
    	double throughputWeight = default(1);
     	

}
