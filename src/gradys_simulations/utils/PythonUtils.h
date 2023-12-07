#ifndef gradys_simulations_PYTHON_UTILS_H_
#define gradys_simulations_PYTHON_UTILS_H_

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include "gradys_simulations/protocols/messages/network/PythonMessage_m.h"
#include "pybind11_json/pybind11_json.hpp"
#include "nlohmann/json.hpp"
#include "gradys_simulations/protocols/messages/internal/PythonMobilityCommand_m.h"

using namespace pybind11::literals;

namespace py = pybind11;

namespace gradys_simulations {

static CommunicationCommand* transformToCommunicationCommandPython(
        py::object comm_command, const char * target) {

    nlohmann::json jsonMessage = nlohmann::json::parse(
            comm_command.attr("message").cast<std::string>());

    // Create communication command
    CommunicationCommand *command = new CommunicationCommand();

    py::object CommunicationCommandTypePython = py::module_::import(
            "gradysim.protocol.messages.communication").attr(
            "CommunicationCommandType");

    py::object ctl = CommunicationCommandTypePython(
            comm_command.attr("command_type").cast<int>());

    if (ctl.is(CommunicationCommandTypePython.attr("SEND"))) {
        command->setCommandType(CommunicationCommandType::SEND_MESSAGE);
//        int target = jsonMessage["destination"];
//        const char* targetName = getSimulation()->getModule(target)->getName();
        command->setTarget(target);
//        targetCommand->setTarget(pk->getName())
    } else if (ctl.is(CommunicationCommandTypePython.attr("BROADCAST"))) {
        command->setCommandType(CommunicationCommandType::SEND_MESSAGE);
        // Broadcast
        command->setTarget(nullptr);
    } else {
        std::cout
                << "Something is wrong in transformToCommunicationCommandPython"
                << std::endl;
        exit(1);
    }

    // Create payload
    PythonMessage *payload = new PythonMessage();
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    payload->setInformation(jsonMessage.dump());

    command->setPayloadTemplate(payload);

    return command;
}

static PythonMobilityCommand* transformToMobilityCommandPython(
        py::object mob_command) {

    // Create mobility command
    PythonMobilityCommand *command = new PythonMobilityCommand();

    py::object MobilityCommandTypePython = py::module_::import(
            "gradysim.protocol.messages.mobility").attr("MobilityCommandType");

    py::object ctl = MobilityCommandTypePython(
            mob_command.attr("command_type").cast<int>());
    if (ctl.is(MobilityCommandTypePython.attr("GOTO_COORDS"))) {
        command->setCommandType(PythonMobilityCommandType::GOTO_COORD);

    } else if (ctl.is(MobilityCommandTypePython.attr("GOTO_GEO_COORDS"))) {
        command->setCommandType(PythonMobilityCommandType::GOTO_GEO_COORD);

    } else if (ctl.is(MobilityCommandTypePython.attr("SET_SPEED"))) {
        command->setCommandType(PythonMobilityCommandType::SET_SPEED);

    } else {
        std::cout << "Something is wrong in transformToMobilityCommandPython"
                << std::endl;
        exit(1);
    }

    command->setParam1(mob_command.attr("param_1").cast<double>());
    command->setParam2(mob_command.attr("param_2").cast<double>());
    command->setParam3(mob_command.attr("param_3").cast<double>());
    command->setParam4(mob_command.attr("param_4").cast<double>());
    command->setParam5(mob_command.attr("param_5").cast<double>());
    command->setParam6(mob_command.attr("param_5").cast<double>());

    return command;
}

static std::string concatenate(const std::string &str, int num) {
    std::string numStr = std::to_string(num);

    return str + "_for_" + numStr;
}

}

#endif /* gradys_simulations_PYTHON_UTILS_H_ */
