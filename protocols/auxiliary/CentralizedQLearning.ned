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

package projeto.protocols.auxiliary;

//
// TODO auto-generated module
//
simple CentralizedQLearning
{
    parameters:
        @class(CentralizedQLearning);
        @signal[trainingCost](type=double);
        @statistic[trainingCost](record=vector);
        @statistic[cumTrainCost](source=sum(trainingCost); record=vector);
        // HyperParameters for the Centralized Q-Learning algorithm
        double learningRate = default(0.1);
        double gamma = default(0.99);
        double epsilon_decay = default(0.01);
        
        // Parameters for the simulated scenario
        double timeInterval @unit(s) = default(100ms); // For our MAMDP model time has to be discrete. This parameter specifieshe interval between 
                                                       // each discretized time state.
                                                       
        double trainingTimeout @unit(s) = default(1s); // The maximum interval between training steps
        
        double distanceInterval @unit(m) = default(1m); // For our MAMDP model the location component of the state space has to be discrete. This
        											    // parameter specifies the interval between each discretized location stage.
        
}
