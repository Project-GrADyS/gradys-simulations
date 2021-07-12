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

#ifndef DRONEMOBILITY_H_
#define DRONEMOBILITY_H_

#include "inet/mobility/single/VehicleMobility.h"
#include "inet/common/geometry/common/GeographicCoordinateSystem.h"

using namespace inet;

namespace projeto {


class DroneMobility : public VehicleMobility
{
    protected:
        enum Command { GOTO=16, STOP=19, JUMP=177, TAKEOFF=22, RETURN_LAUNCH=20, YAW=115, REVERSE=120 };
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

        struct DroneStatus {
            simtime_t idleTime;
            bool isIdle = false;
            double currentYaw = -1;
            double currentYawSpeed;
            bool isReversed = false;
            int targetIndex;
        };
        DroneStatus droneStatus;

        std::vector<Instruction> instructions;

        double verticalSpeed;
        double startTime;
        int currentInstructionIndex=0;

    protected:
        virtual void initialize(int stage) override;
        virtual void setInitialPosition() override;
        virtual void readWaypointsFromFile(const char *fileName) override;

        virtual void move() override;
        virtual void orient() override;
        virtual void fly();
        virtual void climb (double targetHeight);
        virtual void nextInstruction();

        virtual void handleMessage(cMessage *message) override;

    public:
        simsignal_t reverseSignalID;
    private:
        Coord tempSpeed;

    private:
        virtual void createWaypoint(double x, double y, double z, IGeographicCoordinateSystem *coordinateSystem);
};

}
#endif /* DRONEMOBILITY_H_ */
