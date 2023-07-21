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

#ifndef __RT_EXPERIMENT_MAVLINKFILEMOBILITY_H_
#define __RT_EXPERIMENT_MAVLINKFILEMOBILITY_H_

#include <omnetpp.h>
#include <gradys_simulations/mobility/base/MAVLinkMobilityBase.h>

namespace gradys_simulations {

class MAVLinkFileMobility : public MAVLinkMobilityBase
{
  protected:
    virtual void initialize(int stage) override;
    virtual void readMissionFromFile();
    virtual void startMission();

  private:
    virtual mavlink_command_long_t vectorToCommand(std::vector<std::string> &vector);
  protected:
    std::string waypointFile;

};

}

#endif
