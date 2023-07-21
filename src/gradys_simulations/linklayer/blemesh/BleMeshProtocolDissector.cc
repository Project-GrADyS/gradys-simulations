//
// Copyright (C) 2018 OpenSim Ltd.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//
// @author: Zoltan Bojthe
//

#include "inet/common/ProtocolGroup.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/dissector/ProtocolDissectorRegistry.h"
#include "gradys_simulations/linklayer/blemesh/BleMeshMac.h"
#include "gradys_simulations/linklayer/blemesh/BleMeshMacHeader_m.h"
#include "gradys_simulations/linklayer/blemesh/BleMeshProtocolDissector.h"

namespace gradys_simulations {

Register_Protocol_Dissector(&BleMeshMac::blemesh, BleMeshProtocolDissector);

void BleMeshProtocolDissector::dissect(Packet *packet, const Protocol *protocol, ICallback& callback) const
{
    const auto& header = packet->popAtFront<BleMeshMacHeader>();
    callback.startProtocolDataUnit(&BleMeshMac::blemesh);
    callback.visitChunk(header, &BleMeshMac::blemesh);
    if (header->getNetworkProtocol() != -1) {
        auto payloadProtocol = ProtocolGroup::getEthertypeProtocolGroup()->getProtocol(header->getNetworkProtocol());
        callback.dissectPacket(packet, payloadProtocol);
    }
//    auto paddingLength = packet->getDataLength();
//    if (paddingLength > b(0)) {
//        const auto& padding = packet->popHeader(paddingLength);
//        callback.visitChunk(padding, &Protocol::ieee802154);
//    }
    callback.endProtocolDataUnit(&BleMeshMac::blemesh);
}

} // namespace inet
