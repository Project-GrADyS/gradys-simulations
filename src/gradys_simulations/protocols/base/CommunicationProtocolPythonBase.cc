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

#include "gradys_simulations/protocols/base/CommunicationProtocolPythonBase.h"

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
#include "gradys_simulations/protocols/ground/PythonGroundProtocol.h"
#include "omnetpp.h"
#include "gradys_simulations/protocols/messages/network/PythonMessage_m.h"
#include <nlohmann/json.hpp>
#include "../../utils/PythonUtils.h"

using namespace pybind11::literals;

namespace py = pybind11;

namespace gradys_simulations {

Define_Module(CommunicationProtocolPythonBase);

void CommunicationProtocolPythonBase::initialize(int stage,
        std::string classType) {
    std::cout << "Initialize " << std::endl;

    CommunicationProtocolBase::initialize(stage);

    pythonInterpreter = Singleton::GetInstance();

    WATCH_MAP(content);

    if (hasGUI()) {
        this->classType = classType;

        updateGUI();
    }
}

void CommunicationProtocolPythonBase::handleMessage(cMessage *msg) {
    std::cout << "Handle message for " << classType << std::endl;

    if (msg->isSelfMessage()) {
        handleTimer(msg);
    } else {
        CommunicationProtocolBase::handleMessage(msg);
    }
}

void CommunicationProtocolPythonBase::handlePacket(Packet *pk) {
    std::cout << "Handle packet for " << classType << std::endl;

    auto message = pk->peekAtBack<PythonMessage>(B(7), 1);

    py::object SimpleMessageL = py::module_::import(
            "simulator.protocols.simple.SimpleMessage").attr("SimpleMessage");

    py::object message_obj = SimpleMessageL(message->getMap());

    instance.attr("set_timestamp")(simTime().dbl());

    py::list consequences = instance.attr("handle_packet")(message_obj);

    for (auto consequence : consequences) {
        dealWithConsequence(consequence.cast<py::object>());
    }
}

void CommunicationProtocolPythonBase::handleTimer(cMessage *msg) {
    std::cout << "Handle timer for " << classType << std::endl;

    pythonInterpreter = Singleton::GetInstance();

    instance.attr("set_timestamp")(simTime().dbl());

    py::dict timer;
    py::list consequences = instance.attr("handle_timer")(timer);

    for (auto consequence : consequences) {
        dealWithConsequence(consequence.cast<py::object>());
    }
}

void CommunicationProtocolPythonBase::handleTelemetry(
        gradys_simulations::Telemetry *telemetry) {
    std::cout << "Handle telemetry for " << classType << std::endl;

    pythonInterpreter = Singleton::GetInstance();

    instance.attr("set_timestamp")(simTime().dbl());

    py::object TelemetryMessageL = py::module_::import(
            "simulator.messages.Telemetry").attr("Telemetry");

    py::object DroneActivityL = py::module_::import(
            "simulator.messages.Telemetry").attr("DroneActivity");

    py::object droneActivityLL;
    switch (telemetry->getDroneActivity()) {
    case DroneActivity::IDLE: {
        droneActivityLL = DroneActivityL.attr("IDLE");
        break;
    }
    case DroneActivity::NAVIGATING: {
        droneActivityLL = DroneActivityL.attr("NAVIGATING");
        break;
    }
    case DroneActivity::REACHED_EDGE: {
        droneActivityLL = DroneActivityL.attr("REACHED_EDGE");
        break;
    }
    case DroneActivity::FOLLOWING_COMMAND: {
        droneActivityLL = DroneActivityL.attr("FOLLOWING_COMMAND");
        break;
    }
    case DroneActivity::RECHARGING: {
        droneActivityLL = DroneActivityL.attr("IDLE");
        break;
    }
    case DroneActivity::SHUTDOWN: {
        droneActivityLL = DroneActivityL.attr("IDLE");
        break;
    }
    default:
        std::cout << "Something is wrong for " << classType << std::endl;
    }

    py::object telemetry_obj = TelemetryMessageL(telemetry->getNextWaypointID(),
            telemetry->getLastWaypointID(), telemetry->getCurrentLat(),
            telemetry->getCurrentLon(), telemetry->getCurrentAlt(),
            telemetry->isReversed(), droneActivityLL);

    py::list consequences = instance.attr("handle_telemetry")(telemetry_obj);

    for (auto consequence : consequences) {
        dealWithConsequence(consequence.cast<py::object>());
    }
}

void CommunicationProtocolPythonBase::finish() {
    std::cout << "Finish for " << classType << std::endl;

    CommunicationProtocolBase::finish();

    instance.attr("set_timestamp")(simTime().dbl());

    py::list consequences = instance.attr("finish")();

    for (auto consequence : consequences) {
        dealWithConsequence(consequence.cast<py::object>());
    }

    pythonInterpreter->TryCloseInstance();
}

constexpr unsigned int str2int(const char *str, int h = 0) {
    return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ str[h];
}

void CommunicationProtocolPythonBase::dealWithConsequence(
        py::object consequence) {
    std::cout << "Deal with consequences for " << classType << std::endl;

    py::tuple c = py::cast<py::tuple>(consequence);

    ConsequenceType ct;
    switch (str2int(c[0].cast<std::string>().c_str())) {
    case str2int("COMMUNICATION"):
        ct = ConsequenceType::COMMUNICATION;
        break;
    case str2int("MOBILITY"):
        ct = ConsequenceType::MOBILITY;
        break;
    case str2int("TIMER"):
        ct = ConsequenceType::TIMER;
        break;
    case str2int("TRACK_VARIABLE"):
        ct = ConsequenceType::TRACK_VARIABLE;
        break;
    default:
        std::cout << "Something is wrong for " << classType << std::endl;
    }

    switch (ct) {
    case ConsequenceType::COMMUNICATION: {
        CommunicationCommand *communicationCommand =
                gradys_simulations::transformToCommunicationCommandPython(
                        c[1].cast<py::object>());

        sendCommand(communicationCommand);
        break;
    }
    case ConsequenceType::MOBILITY: {
        MobilityCommand *mobilityCommand =
                gradys_simulations::transformToMobilityCommandPython(
                        c[1].cast<py::object>());

        sendCommand(mobilityCommand);
        break;
    }
    case ConsequenceType::TRACK_VARIABLE: {
        py::tuple track_variable = py::cast<py::tuple>(c[1]);

        content[track_variable[0].cast<std::string>()] = track_variable[1].cast<
                std::string>();

        if (hasGUI()) {
            updateGUI();
        }

        break;
    }
    case ConsequenceType::TIMER: {
        py::tuple infos = py::cast<py::tuple>(c[1].cast<py::object>());

        scheduleAt(SimTime(py::cast<double>(infos[1])), new cMessage());
        break;
    }
    default:
        std::cout << "Something is wrong for " << classType << std::endl;
    }

}

void CommunicationProtocolPythonBase::updateGUI() {
    cCanvas *canvas = getParentModule()->getParentModule()->getCanvas();
    cTextFigure *textFigure = check_and_cast<cTextFigure*>(
            canvas->getFigure(
                    ("simulationInformation" + this->classType).c_str()));

    nlohmann::json jsonObject;

    if (textFigure->getText() != nullptr
            && strcmp(textFigure->getText(), "") != 0) {
        jsonObject = nlohmann::json::parse(textFigure->getText());
        for (const auto &pair : content) {
            if (this->classType == "GroundStation") {
                jsonObject[concatenate(pair.first, 0)] = pair.second;
            } else {
                jsonObject[concatenate(pair.first,
                        getParentModule()->getIndex())] = pair.second;
            }
        }
    }

    char label[2000];

    sprintf(label, "%s", jsonObject.dump().c_str());

    textFigure->setText(label);
}

} //namespace

