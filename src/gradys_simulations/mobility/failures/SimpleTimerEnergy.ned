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

// Simple energy model that sends the vehicle home or shuts it down using timers 
simple SimpleTimerEnergy extends FailureGeneratorBase {
    parameters:
        @class(SimpleTimerEnergy);
        double batteryShutdownDuration @unit(s);
        double batteryRTLDuration @unit(s);
        double rechargeDuration @unit(s) = default(3s); // Recharges forever if this parameter is less than zero
}