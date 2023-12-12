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

#ifndef PYTHONDRONEMOBILITY_H_
#define PYTHONDRONEMOBILITY_H_

#include "inet/environment/contract/IGround.h"
#include "inet/mobility/base/MovingMobilityBase.h"

using namespace inet;

namespace gradys_simulations {

class PythonDroneMobility: public MovingMobilityBase {
protected:
    enum PythonCommand {
        GOTO_COORDS = 1, GOTO_GEO_COORDS = 2, SET_SPEED = 3,
    };

    struct PythonInstruction {
        PythonCommand command;
        double param1;
        double param2;
        double param3;
        double param4;
        double param5;
        double param6;

        PythonInstruction(PythonCommand command, double param1, double param2,
                double param3, double param4, double param5, double param6) :
                command(command), param1(param1), param2(param2), param3(
                        param3), param4(param4), param5(param5), param6(param6) {
        }

    };

protected:
    PythonInstruction *instruction = nullptr;

    // The ground module given by the "groundModule" parameter, pointer stored for easier access.
    physicalenvironment::IGround *ground = nullptr;

    double speed;
    double heading;
    double angularSpeed;

    double initialX;
    double initialY;
    double initialZ;

    double initialLongitude;
    double initialLatitude;

    Coord targetPos;

    double startTime;
    simtime_t telemetryFrequency;
    cMessage *telemetryTimer = new cMessage();

protected:
    virtual void initialize(int stage) override;
    virtual void setInitialPosition() override;
    virtual void move() override;
    virtual void orient() override;
    virtual void fly();
    virtual void climb (double targetHeight);

protected:
    virtual void handleMessage(cMessage *message) override;
    virtual void sendTelemetry();
};

}
#endif /* PYTHONDRONEMOBILITY_H_ */
