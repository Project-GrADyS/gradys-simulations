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

#include "GenericProtocol.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "inet/applications/base/ApplicationPacket_m.h"

namespace gradys_simulations {

Define_Module(GenericProtocol);

void GenericProtocol::initialize(int stage)
{
    std::cout << "initialize" << std::endl;
}

void GenericProtocol::handleTelemetry(gradys_simulations::Telemetry *telemetry) {
    std::cout << "handleTelemetry" << std::endl;
}

void GenericProtocol::handlePacket(Packet *pk) {
    std::cout << "handlePacket" << std::endl;
}

void GenericProtocol::sendReverseOrder() {
    std::cout << "sendReverseOrder" << std::endl;
}

void GenericProtocol::updatePayload() {
    std::cout << "updatePayload" << std::endl;
}

void GenericProtocol::setTarget(const char *target) {
    std::cout << "setTarget" << std::endl;
}

bool GenericProtocol::isTimedout() {
    std::cout << "isTimedout" << std::endl;
}

void GenericProtocol::resetParameters() {
    std::cout << "resetParameters" << std::endl;
}
} //namespace
