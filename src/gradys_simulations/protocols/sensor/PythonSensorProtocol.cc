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

//    std::string importPath = "simulator.protocols." + protocol + "."
//            + protocolFileName;

//    std::string importPath = "/home/lac/Documents/Gradys/workspace/gradys-sim-prototype/showcases/" + protocol + "/"
//            + protocolFileName;
//
//    pybind11::object protocolMobileClass =
//            pybind11::module_::import(importPath.c_str()).attr(protocolType.c_str());

//    pybind11::object protocolMobileClass = py::eval("import importlib.util"
//                                                    "import sys"
//                                                    "sys.path.append('/home/lac/Documents/Gradys/workspace/gradys-sim-prototype/showcases/simple')"
//                                                    "spec = importlib.util.spec_from_file_location('gradysim.showcases.simple.SimpleProtocolSensor', '/home/lac/Documents/Gradys/workspace/gradys-sim-prototype/showcases/simple/protocol_sensor.py')"
//                                                    "foo = importlib.util.module_from_spec(spec)"
//                                                    "sys.modules['gradysim.showcases.simple.SimpleProtocolSensor'] = foo"
//                                                    "spec.loader.exec_module(foo)"
//                                                    "return foo.SimpleProtocolSensor()", instance).cast<pybind11::object>();
//
//    instance.attr("encapsulate")(protocolMobileClass);

    py::object scope = py::module_::import("__main__").attr("__dict__");

    pybind11::exec("sys.path.append('/home/lac/Documents/Gradys/workspace/gradys-sim-prototype/showcases/simple')", scope);

    pybind11::exec("from protocol_sensor import SimpleProtocolSensor", scope);

    pybind11::object protocolSensorClass = pybind11::eval("SimpleProtocolSensor", scope).cast<pybind11::object>();

    instance.attr("encapsulate")(protocolSensorClass);

    instance.attr("set_timestamp")(simTime().dbl());

    pybind11::list consequences = instance.attr("initialize")(stage);
    for (auto consequence : consequences) {
        dealWithConsequence(consequence.cast<pybind11::object>());
    }
}

}
