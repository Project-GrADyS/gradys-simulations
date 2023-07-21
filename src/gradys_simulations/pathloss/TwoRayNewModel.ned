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


package gradys_simulations.pathloss;

import inet.physicallayer.wireless.common.contract.packetlevel.IPathLoss;
// https://github.com/erihh3/PFC-TwoRayModel
module TwoRayNewModel like IPathLoss
{
    parameters:
        @class(TwoRayNewModel);
        @display("i=block/control");
        double epsilon_r = default(15);
        double sigma = default(0.005); 
        double delta_h = default(0.05);
        string polarization = default("horizontal");
}
