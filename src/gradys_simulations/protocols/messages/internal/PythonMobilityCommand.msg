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

namespace gradys_simulations;

// Commands that the mobility module should be capable of carrying out
enum PythonMobilityCommandType {
   GOTO_COORD = 1;
   GOTO_GEO_COORD = 2;
   SET_SPEED = 3;
}

// Message declaration containing the command Id and its parameters 
message PythonMobilityCommand {
    PythonMobilityCommandType commandType;
    double param1=-1;
    double param2=-1;
    double param3=-1;
    double param4=-1;
    double param5=-1;
    double param6=-1;
}
