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

#ifndef GEOWAYPOINTMOBILITY_H_
#define GEOWAYPOINTMOBILITY_H_

#include "inet/mobility/single/VehicleMobility.h"

using namespace inet;

namespace projeto {


class GeoWaypointMobility : public VehicleMobility
{
    protected:
        enum Command { GOTO=16, STOP=19, JUMP=177 };
        struct Instruction {
            Command command;
            double param1;
            double param2;
            double param3;
            double param4;
            bool autocontinue;
            int waypointIndex;
            int internalCounter = 0;

            Instruction(Command command, double param1, double param2, double param3, double param4, bool autocontinue) :
                command(command),
                param1(param1),
                param2(param2),
                param3(param3),
                param4(param4),
                autocontinue(autocontinue) { };
        };

        std::vector<Instruction> instructions;

        simtime_t idleTime;
        bool isIdle = false;
        int currentInstructionIndex=0;

    protected:
        virtual void initialize(int stage) override;
        virtual void setInitialPosition() override;
        virtual void readWaypointsFromFile(const char *fileName) override;
        virtual void move() override;

    private:
        Coord tempSpeed;
        double climbAngle;

    private:
        virtual void climb ();
        virtual void fly ();
};

}
#endif /* GEOWAYPOINTMOBILITY_H_ */
