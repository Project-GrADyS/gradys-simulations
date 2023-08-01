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

package gradys_simulations.mobility.failures;
import gradys_simulations.mobility.failures.base.FailureGeneratorBase;

// Randomly generates failures that make the vehicle shutdown for a period of time 
simple RandomFailureGenerator extends FailureGeneratorBase
{
    parameters:
        @class(RandomFailureGenerator);
        // No failures will be triggered before this timestamp
        double failureStart @unit(s) =  default(0s);
        // Minimum interval between failures
        double failureMinimumInterval @unit(s) = default(0s);
        // Time between failure rolls
        double failureRollTime @unit(s) = default(1s);
       	// Chance of failure every roll
       	double failureChance;  
       	// Duration of a failure, normal operation will resume afterwards
       	double failureDuration @unit(s) = default(15s);
}
