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
#include "gradys_simulations/utils/ConsequenceType.h"
using namespace pybind11::literals;

namespace py = pybind11;

namespace gradys_simulations {
Define_Module(PythonDroneProtocol);

void PythonDroneProtocol::handleMessage(cMessage *msg) {
    std::cout << "Handle message drone protocol" << std::endl;
    if (msg->isSelfMessage()) {
        handleTimer(msg);
    } else {
        CommunicationProtocolBase::handleMessage(msg);
    }
}

void PythonDroneProtocol::handleTimer(cMessage *msg) {
    std::cout << "Handle timer drone protocol" << std::endl;

    pythonInterpreter = Singleton::GetInstance();

    instance.attr("set_timestamp")(simTime().dbl());

    py::dict timer;
    py::list consequences = instance.attr("handle_timer")(timer);

    std::cout << "List size: " << consequences.size() << std::endl;
    for (auto consequence : consequences) {
        dealWithConsequence(consequence.cast<py::object>());
    }
}

void PythonDroneProtocol::handleTelemetry(
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

void PythonDroneProtocol::initialize(int stage) {
    std::cout << "Initialize drone protocol" << std::endl;

    CommunicationProtocolBase::initialize(stage);

    pythonInterpreter = Singleton::GetInstance();

//    emit(registerSignal("dataLoad"), "");

//    std::string str;
//    for (auto it = content.cbegin(); it != content.cend(); ++it) {
//        str.append("Key: ");
//        str.append(it->first);
//        str.append("Value: ");
//        str.append(it->second);
//        str.append("\n");
//    }
//
//    std::string test = "asdf";
//    char buf[100];
//    sprintf(buf, "%s", test.c_str());
//
//    if (hasGUI()) {
//        char label[50];
//        // Write last hop count to string
//        sprintf(label, "last hopCount");
//        // Get pointer to figure
//        cCanvas *canvas = getParentModule()->getCanvas();
//        cTextFigure *textFigure = check_and_cast<cTextFigure*>(
//                canvas->getFigure("lasthopcount"));
//        // Update figure text
//        textFigure->setText(label);
//    }

    WATCH_MAP(content);

    py::object InteropEncapsulator = py::module_::import(
            "simulator.encapsulator.InteropEncapsulator").attr(
            "InteropEncapsulator");

    py::object SimpleProtocolMobile = py::module_::import(
            "simulator.protocols.simple.SimpleProtocolMobile").attr(
            "SimpleProtocolMobile");
    instance = InteropEncapsulator.attr("encapsulate")(SimpleProtocolMobile);

    instance.attr("set_timestamp")(simTime().dbl());

    py::list consequences = instance.attr("initialize")(stage);

    std::cout << "Initialize drone protocol list size: " << consequences.size()
            << std::endl;
    for (auto consequence : consequences) {
        dealWithConsequence(consequence.cast<py::object>());
    }
}

void PythonDroneProtocol::dealWithConsequence(py::object consequence) {
    std::cout << "Deal with consequence drone protocol" << std::endl;

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

//        std::string str;
//        for (auto it = content.cbegin(); it != content.cend(); ++it) {
//            str.append("Key: ");
//            str.append(it->first);
//            str.append("Value: ");
//            str.append(it->second);
//            str.append("\n");
//        }
//
//        std::string test = "asdf";
//        char buf[100];
//        sprintf(buf, "%s", test.c_str());

//        int hopcount = 1;
//
//        if (hasGUI()) {
//                char label[50];
//                // Write last hop count to string
//                sprintf(label, "last hopCount = %d", hopcount);
//                // Get pointer to figure
//                cCanvas *canvas = getParentModule()->getCanvas();
//                cTextFigure *textFigure = check_and_cast<cTextFigure*>(canvas->getFigure("lasthopcount"));
//                // Update figure text
//                textFigure->setText(label);
//        }

//        emit(registerSignal("dataLoad"), "asdf");

        content[key.cast<std::string>()] = value.cast<std::string>();

    } else if (ct == ConsequenceType::TIMER) {
        py::object timer_command = c[1];
        py::print(timer_command);
        py::tuple infos = py::cast<py::tuple>(timer_command);

        timer = new cMessage();
        scheduleAt(SimTime(py::cast<double>(infos[1])), timer);
    }

}

void PythonDroneProtocol::handlePacket(Packet *pk) {
    std::cout << "Handle packet drone protocol" << std::endl;

    auto message = pk->peekAtBack<SimpleMessage>(B(7), 1);

    py::object SimpleMessageL = py::module_::import(
            "simulator.protocols.simple.SimpleMessage").attr("SimpleMessage");

    py::object message_obj = SimpleMessageL(
            "sender"_a = gradys_simulations::getSenderType(
                    static_cast<int>(message->getSenderType())), "content"_a =
                    message->getContent());

    instance.attr("set_timestamp")(simTime().dbl());

    py::list consequences = instance.attr("handle_packet")(message_obj);

    std::cout << "List size: " << consequences.size() << std::endl;
    for (auto consequence : consequences) {
        dealWithConsequence(consequence.cast<py::object>());
    }
}

void PythonDroneProtocol::finish() {
    std::cout << "Finish drone protocol" << std::endl;

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
} /* namespace gradys_simulations */
