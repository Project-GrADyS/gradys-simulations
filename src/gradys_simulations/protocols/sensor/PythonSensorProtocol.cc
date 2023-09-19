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

#include <nlohmann/json.hpp>
#include <pybind11/pybind11.h>

#include "gradys_simulations/protocols/messages/network/PythonMessage_m.h"
#include "gradys_simulations/protocols/sensor/PythonSensorProtocol.h"

using namespace pybind11::literals;

namespace gradys_simulations {
Define_Module(PythonSensorProtocol);

void PythonSensorProtocol::initialize(int stage) {
    CommunicationProtocolPythonBase::initialize(stage, "Sensors");

    protocol = par("protocol").stringValue();
    protocolFileName = par("protocolFileName").stringValue();
    protocolType = par("protocolSensor").stringValue();

    pybind11::object InteropEncapsulator = pybind11::module_::import(
            "simulator.encapsulator.interop").attr("InteropEncapsulator");
    instance = InteropEncapsulator();

    std::string importPath = "simulator.protocols." + protocol + "."
            + protocolFileName;
    pybind11::object protocolMobileClass =
            pybind11::module_::import(importPath.c_str()).attr(protocolType.c_str());
    instance.attr("encapsulate")(protocolMobileClass);

    instance.attr("set_timestamp")(simTime().dbl());

    pybind11::list consequences = instance.attr("initialize")(stage);
    for (auto consequence : consequences) {
        dealWithConsequence(consequence.cast<pybind11::object>());
    }
}

}
