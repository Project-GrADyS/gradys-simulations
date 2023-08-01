#include <vector>
#include <memory>
#include "VehicleTypes.h"
#include "gradys_simulations/mobility/base/MAVLinkInstruction.h"
#include "inet/common/geometry/common/GeographicCoordinateSystem.h"
#include "VehicleModes.h"

namespace gradys_simulations {
// 
// This module provides several utility functions that facilitate the communication
// with the SITL instance. Each function generates a vector of instructions that 
// command the simulated vehicle to perform a common task. This vector of instructions
// can be read by [MAVLinkMobilityBase](/MAVSIMNET/Modules/MAVLinkMobilityBase/) with the
// queueInstructions() method.
namespace VehicleRoutines {

// Instructs the vehicle to ARM and TAKEOFF
std::vector<std::shared_ptr<Instruction>> armTakeoff(uint8_t senderSystem, uint8_t senderComponent, VehicleType type, float altitude, uint8_t targetSystem, uint8_t targetComponent, omnetpp::simtime_t timeout=60, int retries=3);

// Instructs the vehicle to set a specific Mode
std::vector<std::shared_ptr<Instruction>> setMode(uint8_t senderSystem, uint8_t senderComponent, VehicleType type, Mode mode, uint8_t targetSystem, uint8_t targetComponent, omnetpp::simtime_t timeout=60, int retries=3);

// Instructs the vehicle to goto a certain location
std::vector<std::shared_ptr<Instruction>> guidedGoto(VehicleType type, double latitude, double longitude, float altitude, double tolerance,
        inet::IGeographicCoordinateSystem *coordinateSystem, uint8_t targetSystem, uint8_t targetComponent, omnetpp::simtime_t timeout=60, int retries=3);
}
}
