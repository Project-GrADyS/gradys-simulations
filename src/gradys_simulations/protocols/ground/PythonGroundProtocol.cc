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

#include "PythonGroundProtocol.h"

#include "inet/common/ModuleAccess.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "inet/applications/base/ApplicationPacket_m.h"
#include <pybind11/pybind11.h>
#include "gradys_simulations/utils/ConsequenceType.h"
#include "gradys_simulations/protocols/ground/PythonGroundProtocol.h"
#include "omnetpp.h"
#include "gradys_simulations/protocols/messages/network/PythonMessage_m.h"
#include <nlohmann/json.hpp>

using namespace pybind11::literals;

namespace py = pybind11;

namespace gradys_simulations {
Define_Module(PythonGroundProtocol);

void PythonGroundProtocol::handleMessage(cMessage *msg) {
    std::cout << "Handle message ground protocol" << std::endl;
    if (msg->isSelfMessage()) {
        handleTimer(msg);
    } else {
        CommunicationProtocolBase::handleMessage(msg);
    }
}

void PythonGroundProtocol::handleTimer(cMessage *msg) {
    std::cout << "Handle timer ground protocol" << std::endl;

    pythonInterpreter = Singleton::GetInstance();

    instance.attr("set_timestamp")(simTime().dbl());

    py::dict timer;
    py::list consequences = instance.attr("handle_timer")(timer);

    std::cout << "List size: " << consequences.size() << std::endl;
    for (auto consequence : consequences) {
        dealWithConsequence(consequence.cast<py::object>());
    }
}

void PythonGroundProtocol::handleTelemetry(
        gradys_simulations::Telemetry *telemetry) {
    std::cout << "Handle telemetry drone protocol" << std::endl;

    pythonInterpreter = Singleton::GetInstance();

    instance.attr("set_timestamp")(simTime().dbl());

    py::object TelemetryMessageL = py::module_::import(
            "simulator.messages.Telemetry").attr("Telemetry");

    py::object DroneActivityL = py::module_::import(
            "simulator.messages.Telemetry").attr("DroneActivity");

    py::object droneActivityLL;
    if (telemetry->getDroneActivity() == DroneActivity::IDLE) {
        droneActivityLL = DroneActivityL.attr("IDLE");
    } else if (telemetry->getDroneActivity() == DroneActivity::NAVIGATING) {
        droneActivityLL = DroneActivityL.attr("NAVIGATING");
    } else if (telemetry->getDroneActivity() == DroneActivity::REACHED_EDGE) {
        droneActivityLL = DroneActivityL.attr("REACHED_EDGE");
    } else if (telemetry->getDroneActivity()
            == DroneActivity::FOLLOWING_COMMAND) {
        droneActivityLL = DroneActivityL.attr("FOLLOWING_COMMAND");
    } else if (telemetry->getDroneActivity() == DroneActivity::RECHARGING) {
        droneActivityLL = DroneActivityL.attr("IDLE");
    } else if (telemetry->getDroneActivity() == DroneActivity::SHUTDOWN) {
        droneActivityLL = DroneActivityL.attr("IDLE");
    }

    py::object telemetry_obj = TelemetryMessageL(telemetry->getNextWaypointID(),
            telemetry->getLastWaypointID(), telemetry->getCurrentLat(),
            telemetry->getCurrentLon(), telemetry->getCurrentAlt(),
            telemetry->isReversed(), droneActivityLL);

    py::list consequences = instance.attr("handle_telemetry")(telemetry_obj);

    std::cout << "List size: " << consequences.size() << std::endl;
    for (auto consequence : consequences) {
        dealWithConsequence(consequence.cast<py::object>());
    }
}

void PythonGroundProtocol::initialize(int stage) {
    std::cout << "Initialize ground protocol" << std::endl;

    CommunicationProtocolBase::initialize(stage);

    pythonInterpreter = Singleton::GetInstance();

    if (hasGUI()) {
        cCanvas *canvas = getParentModule()->getParentModule()->getCanvas();
        cTextFigure *textFigure = check_and_cast<cTextFigure*>(
                canvas->getFigure("simulationInformationGroundStation"));

        nlohmann::json jsonObjectOutgoing;

        char label[1000];
        // Write last hop count to string
        sprintf(label, "%s", jsonObjectOutgoing.dump().c_str());

        // Update figure text
        textFigure->setText(label);
    }

    WATCH_MAP(content);

    py::object InteropEncapsulator = py::module_::import(
            "simulator.encapsulator.InteropEncapsulator").attr(
            "InteropEncapsulator");

    py::object SimpleProtocolGround = py::module_::import(
            "simulator.protocols.simple.SimpleProtocolGround").attr(
            "SimpleProtocolGround");
    instance = InteropEncapsulator.attr("encapsulate")(SimpleProtocolGround);

    instance.attr("set_timestamp")(simTime().dbl());

    py::list consequences = instance.attr("initialize")(stage);

    std::cout << "List size: " << consequences.size() << std::endl;
    for (auto consequence : consequences) {
        dealWithConsequence(consequence.cast<py::object>());
    }
}

void PythonGroundProtocol::dealWithConsequence(py::object consequence) {
    std::cout << "Deal with consequence ground protocol" << std::endl;

    py::tuple c = py::cast<py::tuple>(consequence);
    ConsequenceType ct = gradys_simulations::transformToConsequenceTypePython(
            c[0].cast<std::string>());

    if (ct == ConsequenceType::COMMUNICATION) {
        py::object comm_command = c[1];

        communicationCommand =
                gradys_simulations::transformToCommunicationCommandPython(
                        comm_command);

        sendCommand(communicationCommand);

    } else if (ct == ConsequenceType::MOBILITY) {
        py::object mob_command = c[1];

        mobilityCommand = gradys_simulations::transformToMobilityCommandPython(
                mob_command);

        sendCommand(mobilityCommand);

    } else if (ct == ConsequenceType::TRACK_VARIABLE) {
        py::tuple track_variable = py::cast<py::tuple>(c[1]);

        py::object key = track_variable[0];
        py::object value = track_variable[1];

        if (hasGUI()) {
            cCanvas *canvas = getParentModule()->getParentModule()->getCanvas();
            cTextFigure *textFigure = check_and_cast<cTextFigure*>(
                    canvas->getFigure("simulationInformationGroundStation"));

            nlohmann::json jsonObjectOutgoing = nlohmann::json::parse(
                    textFigure->getText());
            for (const auto &pair : content) {
                jsonObjectOutgoing[concatenate(pair.first, 0)] = pair.second;
            }

            char label[1000];
            // Write last hop count to string
            sprintf(label, "%s", jsonObjectOutgoing.dump().c_str());

            // Update figure text
            textFigure->setText(label);
        }

        content[key.cast<std::string>()] = value.cast<std::string>();

    } else if (ct == ConsequenceType::TIMER) {
        py::object timer_command = c[1];
        py::print(timer_command);
        py::tuple infos = py::cast<py::tuple>(timer_command);

        cMessage *timer = new cMessage();
        scheduleAt(SimTime(py::cast<double>(infos[1])), timer);
    }

}

void PythonGroundProtocol::handlePacket(Packet *pk) {
    std::cout << "Handle packet ground protocol" << std::endl;

    auto message = pk->peekAtBack<PythonMessage>(B(7), 1);

    py::object SimpleMessageL = py::module_::import(
            "simulator.protocols.simple.SimpleMessage").attr("SimpleMessage");

    py::dict messageDict = message->getMap();

    py::object message_obj = SimpleMessageL(messageDict);

    instance.attr("set_timestamp")(simTime().dbl());

    py::list consequences = instance.attr("handle_packet")(message_obj);

    std::cout << "List size: " << consequences.size() << std::endl;
    for (auto consequence : consequences) {
        dealWithConsequence(consequence.cast<py::object>());
    }
}

void PythonGroundProtocol::finish() {
    std::cout << "Finish ground protocol" << std::endl;

    CommunicationProtocolBase::finish();

    instance.attr("set_timestamp")(simTime().dbl());

    py::list consequences = instance.attr("finish")();

    std::cout << "List size: " << consequences.size() << std::endl;
    for (auto consequence : consequences) {
        dealWithConsequence(consequence.cast<py::object>());
    }

    if (communicationCommand != nullptr
            && communicationCommand->isSelfMessage()) {
        cancelAndDelete(communicationCommand);
    }
    if (mobilityCommand != nullptr && mobilityCommand->isSelfMessage()) {
        cancelAndDelete(mobilityCommand);
    }
    if (timer != nullptr && timer->isSelfMessage()) {
        cancelAndDelete(timer);
    }

    pythonInterpreter->TryCloseInstance();
}
/* namespace gradys_simulations */
}
