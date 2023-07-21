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

#ifndef DRONEMOBILITYMAV_H_
#define DRONEMOBILITYMAV_H_

#include <queue>
#include "gradys_simulations/mobility/base/MAVLinkMobilityBase.h"
#include "inet/common/geometry/common/GeographicCoordinateSystem.h"

#include "gradys_simulations/protocols/messages/internal/MobilityCommand_m.h"
#include "gradys_simulations/protocols/messages/internal/Telemetry_m.h"

using namespace inet;

namespace gradys_simulations {


class DroneMobilityMav : public MAVLinkMobilityBase
{
    protected:
        // Tipos de comandos implementados
        enum Command { GOTO=16, STOP=19, JUMP=177, TAKEOFF=22, RETURN_LAUNCH=20, YAW=115, REVERSE=120 };

        // Struct que representa uma instru��o lida do arquivo de instru��es
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

        // Control struct
        struct DroneStatus {
            // Start of last timeout instruction
            simtime_t idleTime;
            bool isIdle = false;

            bool isReversed = false;

            // Current Waypoint (VehicleMobility structure)
            int targetIndex = {};

            // Saves the last target index
            int lastInstructionIndex = {};

            /* Command status */
            // Current MobilityCommunicationCommand being followed
            int currentCommand=-1;
            MobilityCommand currentCommandInstance = {};
            // Target waypoint for commands that use it
            int gotoWaypointTarget=-1;

            // Queue of commands that will execute in order they are recieved
            std::queue<MobilityCommand*> commandQueue;

            DroneActivity currentActivity = {};
        };
        DroneStatus droneStatus;

        std::vector<Instruction> instructions;

        double speed;

        double startTime;

        int currentInstructionIndex=0;

        Coord homeCoords;

        double waypointProximity;

        std::vector<Coord> waypoints;

        IGeographicCoordinateSystem *coordinateSystem;

        simtime_t telemetryFrequency;
        cMessage *telemetryTimer = new cMessage();

    protected:
        virtual void initialize(int stage) override;
        virtual void readWaypointsFromFile(const char *fileName);

        // Fun��o de movimento chamada periodicamente em um self-timeout
        virtual void move() override;

        // Fun��o de voo respons�vel por definir a posi��o plana do drone
        virtual void fly();

        // Fun��o que pula para pr�xima instru��o
        virtual void nextInstruction();

        // Fun��o que captura ordens recebidas pela gate de input de ordem e trata
        virtual void handleMessage(cMessage *message) override;

        // Checks if current command has finished and performs next command in queue
        virtual void executeCommand();

        virtual ~DroneMobilityMav();
    public:
        simsignal_t reverseSignalID;
    private:
        Coord tempSpeed;
        Coord MAVLinkCurrentCoord;
        bool hasTakenOff = false;

    private:
        virtual void createWaypoint(double x, double y, double z, IGeographicCoordinateSystem *coordinateSystem);
        // Sends telemetry to the output gate
        virtual void sendTelemetry(bool sendTour = false);

        virtual int instructionIndexFromWaypoint(int waypointIndex);
};

}
#endif /* DRONEMOBILITYMAV_H_ */
