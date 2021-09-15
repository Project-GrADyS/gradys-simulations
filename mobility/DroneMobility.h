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

#include <queue>
#include "inet/mobility/single/VehicleMobility.h"
#include "inet/common/geometry/common/GeographicCoordinateSystem.h"

#include "../communication/messages/internal/MobilityCommand_m.h"
#include "../communication/messages/internal/Telemetry_m.h"

using namespace inet;

namespace projeto {


class DroneMobility : public VehicleMobility
{
    protected:
        // Tipos de comandos implementados
        enum Command { GOTO=16, STOP=19, JUMP=177, TAKEOFF=22, RETURN_LAUNCH=20, YAW=115, REVERSE=120 };

        // Struct que representa uma instrução lida do arquivo de instruções
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

        // Struct de controle de estado
        struct DroneStatus {
            // Tempo de início de uma instrução de espera
            simtime_t idleTime;
            // Booleando que indica se está acontecendo uma espera
            bool isIdle = false;

            // Variável que guarda o yaw definido por uma instrução de yaw
            double currentYaw = -1;
            // Velocidade atual de yaw
            double currentYawSpeed;

            // Controle de estado de reversão do trone
            bool isReversed = false;

            // Waypoint (estrutura do VehicleMobility) alvo atual
            int targetIndex;

            // Saves the last target index
            int lastInstructionIndex;

            /* Command statuses */
            // Current MobilityCommunicationCommand being followed
            int currentCommand=-1;
            int gotoWaypointTarget=-1;

            // Queue of commands that will execute in order they are recieved
            std::queue<MobilityCommand*> commandQueue;

            DroneActivity currentActivity;
        };
        DroneStatus droneStatus;

        // Vetor de instruções
        std::vector<Instruction> instructions;

        // Variáveis que guardam valores da ned file
        double verticalSpeed;
        double startTime;

        // Variável de controle da instrução sendo executada atualmente
        int currentInstructionIndex=0;

    protected:
        virtual void initialize(int stage) override;
        virtual void setInitialPosition() override;
        virtual void readWaypointsFromFile(const char *fileName) override;

        // Função de movimento chamada periodicamente em um self-timeout
        virtual void move() override;

        // Função de orientação chamada periodicamente em um self-timeout
        virtual void orient() override;

        // Função de voo responsável por definir a posição plana do drone
        virtual void fly();

        // Função de voo responsável por definir a posição vertical do drone
        virtual void climb (double targetHeight);

        // Função que pula para próxima instrução
        virtual void nextInstruction();

        // Função que captura ordens recebidas pela gate de input de ordem e trata
        virtual void handleMessage(cMessage *message) override;

        // Checks if current command has finished and performs next command in queue
        virtual void executeCommand();

    public:
        simsignal_t reverseSignalID;
    private:
        Coord tempSpeed;

    private:
        virtual void createWaypoint(double x, double y, double z, IGeographicCoordinateSystem *coordinateSystem);
        // Sends telemetry to the output gate
        virtual void sendTelemetry(bool sendTour = false);

        virtual int instructionIndexFromWaypoint(int waypointIndex);
};

}
#endif /* DRONEMOBILITY_H_ */
