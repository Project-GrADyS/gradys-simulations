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

namespace py = pybind11;

namespace gradys_simulations {
Define_Module(PythonGroundProtocol);

void PythonGroundProtocol::handleMessage(cMessage *msg) {
    if (msg->isSelfMessage()) {
        handleTimer(msg);
    } else {
        CommunicationProtocolBase::handleMessage(msg);
    }
}

void PythonGroundProtocol::handleTimer(cMessage *msg) {
    pythonInterpreter = Singleton::GetInstance();

    py::dict timer;
    py::list consequences = instance.attr("handle_timer")(timer);

    std::cout << "List size: " << consequences.size() << std::endl;
    for (auto consequence : consequences) {
        py::tuple c = py::cast<py::tuple>(consequence);
        ConsequenceType ct =
                gradys_simulations::transformToConsequenceTypePython(
                        c[0].cast<std::string>());

        if (ct == ConsequenceType::COMMUNICATION) {
            py::object comm_command = c[1];

            CommunicationCommand *command =
                    gradys_simulations::transformToCommunicationCommandPython(
                            comm_command);

            sendCommand(command);

        } else if (ct == ConsequenceType::MOBILITY) {
            py::object mob_command = c[1];

            MobilityCommand *command =
                    gradys_simulations::transformToMobilityCommandPython(
                            mob_command);

            sendCommand(command);

        } else if (ct == ConsequenceType::TIMER) {
            py::object timer_command = c[1];
            py::print(timer_command);
            py::tuple infos = py::cast<py::tuple>(timer_command);

            cMessage *timer = new cMessage();
            scheduleAfter(SimTime(py::cast<double>(infos[1])), timer);
        }
    }
}

void PythonGroundProtocol::initialize(int stage) {
    pythonInterpreter = Singleton::GetInstance();

    py::object InteropEncapsulator = py::module_::import(
            "simulator.encapsulator.InteropEncapsulator").attr(
            "InteropEncapsulator");

    py::object SimpleProtocolGround = py::module_::import(
            "simulator.protocols.simple.SimpleProtocolGround").attr(
            "SimpleProtocolGround");
    instance = InteropEncapsulator.attr("encapsulate")(SimpleProtocolGround);

    py::list consequences = instance.attr("initialize")(stage);

    std::cout << "List size: " << consequences.size() << std::endl;
    for (auto consequence : consequences) {
        py::tuple c = py::cast<py::tuple>(consequence);
        ConsequenceType ct =
                gradys_simulations::transformToConsequenceTypePython(
                        c[0].cast<std::string>());

        if (ct == ConsequenceType::COMMUNICATION) {
            py::object comm_command = c[1];

            CommunicationCommand *command =
                    gradys_simulations::transformToCommunicationCommandPython(
                            comm_command);

            sendCommand(command);

        } else if (ct == ConsequenceType::MOBILITY) {
            py::object mob_command = c[1];

            MobilityCommand *command =
                    gradys_simulations::transformToMobilityCommandPython(
                            mob_command);

            sendCommand(command);

        } else if (ct == ConsequenceType::TIMER) {
            py::object timer_command = c[1];
            py::print(timer_command);
            py::tuple infos = py::cast<py::tuple>(timer_command);

            cMessage *timer = new cMessage();
            scheduleAfter(SimTime(py::cast<double>(infos[1])), timer);
        }
    }
}

void PythonGroundProtocol::handlePacket(Packet *pk) {
    auto message = pk->peekAtBack<SimpleMessage>(B(7), 1);

    py::object SimpleMessageT = py::module_::import(
            "simulator.protocols.simple.SimpleMessage").attr("SimpleMessage");

    py::object message_obj = SimpleMessageT(
            gradys_simulations::getSenderType(
                    static_cast<int>(message->getSenderType())),
            message->getContent());

    py::list consequences = instance.attr("handle_packet")(message_obj);

    std::cout << "List size: " << consequences.size() << std::endl;
    for (auto consequence : consequences) {
        py::tuple c = py::cast<py::tuple>(consequence);
        ConsequenceType ct =
                gradys_simulations::transformToConsequenceTypePython(
                        c[0].cast<std::string>());

        if (ct == ConsequenceType::COMMUNICATION) {
            py::object comm_command = c[1];

            CommunicationCommand *command =
                    gradys_simulations::transformToCommunicationCommandPython(
                            comm_command);

            sendCommand(command);

        } else if (ct == ConsequenceType::MOBILITY) {
            py::object mob_command = c[1];

            MobilityCommand *command =
                    gradys_simulations::transformToMobilityCommandPython(
                            mob_command);

            sendCommand(command);

        } else if (ct == ConsequenceType::TIMER) {
            py::object timer_command = c[1];
            py::print(timer_command);
            py::tuple infos = py::cast<py::tuple>(timer_command);

            cMessage *timer = new cMessage();
            scheduleAfter(SimTime(py::cast<double>(infos[1])), timer);
        }
    }
    std::cout << "Test: " << consequences.size() << std::endl;
} /* namespace gradys_simulations */

}
