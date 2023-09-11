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
#include <pybind11/operators.h>
#include <nlohmann/json.hpp>
#include "../../utils/PythonUtils.h"
#include "gradys_simulations/protocols/messages/network/PythonMessage_m.h"

using namespace pybind11::literals;

namespace py = pybind11;

namespace gradys_simulations {
Define_Module(PythonDroneProtocol);

void PythonDroneProtocol::initialize(int stage) {
    CommunicationProtocolPythonBase::initialize(stage, "Drones");

    py::object InteropEncapsulator = py::module_::import(
            "simulator.encapsulator.InteropEncapsulator").attr(
            "InteropEncapsulator");

    py::object SimpleProtocolMobile = py::module_::import(
            "simulator.protocols.simple.SimpleProtocolMobile").attr(
            "SimpleProtocolMobile");
    instance = InteropEncapsulator.attr("encapsulate")(SimpleProtocolMobile);

    instance.attr("set_timestamp")(simTime().dbl());

    py::list consequences = instance.attr("initialize")(stage);
    for (auto consequence : consequences) {
        dealWithConsequence(consequence.cast<py::object>());
    }
}

} /* namespace gradys_simulations */
