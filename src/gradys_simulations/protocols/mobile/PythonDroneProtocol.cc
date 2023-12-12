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

namespace gradys_simulations {
Define_Module(PythonDroneProtocol);

void PythonDroneProtocol::initialize(int stage) {
    CommunicationProtocolPythonBase::initialize(stage, "Drones");

    if(stage == INITSTAGE_LOCAL) {
        customProtocolLocation = par("customProtocolLocation").stringValue();
        protocol = par("protocol").stringValue();
        protocolFileName = par("protocolFileName").stringValue();
        protocolType = par("protocolMobile").stringValue();

        pybind11::object InteropEncapsulator = pybind11::module_::import(
                "gradysim.encapsulator.interop").attr("InteropEncapsulator");
        instance = InteropEncapsulator();

        py::object scope = py::module_::import("__main__").attr("__dict__");

        std::string sysPath = "sys.path.append('" + customProtocolLocation + "/"
                + protocol + "')";
        pybind11::exec(sysPath, scope);

        std::string import = "from " + protocolFileName + " import " + protocolType;
        pybind11::exec(import, scope);

        pybind11::object protocolMobileClass =
                pybind11::eval(protocolType, scope).cast<pybind11::object>();

        instance.attr("encapsulate")(protocolMobileClass);

        instance.attr("set_timestamp")(simTime().dbl());

        instance.attr("set_id")(this->getParentModule()->getId());

        pybind11::list consequences = instance.attr("initialize")();
        for (auto consequence : consequences) {
            dealWithConsequence(consequence.cast<pybind11::object>(), nullptr);
        }

    }
}

} /* namespace gradys_simulations */
