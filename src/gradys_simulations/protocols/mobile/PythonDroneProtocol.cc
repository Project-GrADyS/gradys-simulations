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

#include "PythonDroneProtocol.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/packet/Packet.h"
#include <pybind11/pybind11.h>

using namespace pybind11::literals;

namespace gradys_simulations {
Define_Module(PythonDroneProtocol);



void PythonDroneProtocol::initialize(int stage) {
    py::scoped_interpreter guard{};

    py::object InteropEncapsulator = py::module_::import("simulator.encapsulator.InteropEncapsulator").attr("InteropEncapsulator");

    py::object SimpleProtocolMobile = py::module_::import("simulator.protocols.simple.SimpleProtocolMobile").attr("SimpleProtocolMobile");
    instance = InteropEncapsulator.attr("encapsulate")(SimpleProtocolMobile);

    py::list consequences = instance.attr("initialize")(stage);

    std::cout << consequences.size() << std::endl;
    for(auto consequence: consequences) {
        py::print(consequence);
    }

}

void PythonDroneProtocol::handlePacket(Packet *pk) {
    py::scoped_interpreter guard {};
    auto message = pk->peekAtBack<SimpleMessage>(B(7), 1);

    py::object SenderType = py::module_::import("simulator.protocols.simple.SimpleMessage").attr("SenderType");
    py::object SimpleMessage = py::module_::import("simulator.protocols.simple.SimpleMessage").attr("SimpleMessage");

    py::object message_obj = SimpleMessage("sender"_a=static_cast<int>(message->getSenderType()), "content"_a=message->getContent());

    py::list consequences = instance.attr("handle_packet")(message_obj);

    std::cout << consequences.size() << std::endl;
    for(auto consequence: consequences) {
        py::print(consequence);
    }
}

void PythonDroneProtocol::finish() {
    py::scoped_interpreter guard {};
    py::list consequences = instance.attr("finish")();

    std::cout << consequences.size() << std::endl;
    for(auto consequence: consequences) {
        py::print(consequence);
    }
}
} /* namespace gradys_simulations */
