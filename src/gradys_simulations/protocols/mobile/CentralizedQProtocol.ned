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

package gradys_simulations.protocols.mobile;

import gradys_simulations.protocols.base.CommunicationProtocolBase;

simple CentralizedQProtocol extends CommunicationProtocolBase
{
    parameters:
        @class(CentralizedQProtocol);
        @signal[dataLoad](type=long);
        @statistic[dataLoad](record=vector,last); // Declaration of dataLoad signal used to track current data load that the drone is carrying

        // Interval at which general REQUEST messages are sent to the network
        double requestInterval @unit(s) = default(1s);

        // Delay between communication commands and actual messages being sent. Actual
        // delay will be exponentially close to this, randomized to prevent collision
        double communicationDelay @unit(s) = default(100ms);

        int packetLimit = default(1000); // Limit of packets that this UAV can carry
}
