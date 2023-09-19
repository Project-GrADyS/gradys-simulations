/*
 * VehicleTypes.h
 *
 *  Created on: 12 de mar de 2022
 *      Author: thlam
 */

#ifndef gradys_simulations_PYTHON_UTILS_H_
#define gradys_simulations_PYTHON_UTILS_H_

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include "gradys_simulations/protocols/messages/network/PythonMessage_m.h"
#include "pybind11_json/pybind11_json.hpp"
#include "nlohmann/json.hpp"

using namespace pybind11::literals;

namespace py = pybind11;

namespace gradys_simulations {

enum class ConsequenceType {
    COMMUNICATION = 1, MOBILITY = 2, TIMER = 3, TRACK_VARIABLE = 4
};

static CommunicationCommand* transformToCommunicationCommandPython(
        py::object comm_command) {

    nlohmann::json jsonMessage = nlohmann::json::parse(
            comm_command.attr("message").cast<std::string>());

    PythonMessage *payload = new PythonMessage();
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());

    payload->setInformation(jsonMessage.dump());

    // Sends command to the communication module to start using this message
    CommunicationCommand *command = new CommunicationCommand();

    py::object CommunicationCommandTypeL = py::module_::import(
            "simulator.messages.communication").attr(
            "CommunicationCommandType");

    py::object ctl = CommunicationCommandTypeL(
            comm_command.attr("command").cast<int>());

    if (ctl.is(CommunicationCommandTypeL.attr("SEND"))) {
        command->setCommandType(CommunicationCommandType::SEND_MESSAGE);

    } else if (ctl.is(CommunicationCommandTypeL.attr("BROADCAST"))) {
        command->setCommandType(CommunicationCommandType::SET_TARGET);

    } else {
        std::cout
                << "Something is wrong for transformToCommunicationCommandPython"
                << std::endl;
    }

    command->setPayloadTemplate(payload);

    return command;
}

static MobilityCommand* transformToMobilityCommandPython(
        py::object mob_command) {
    MobilityCommand *command = new MobilityCommand();

    py::object MobilityCommandTypeL = py::module_::import(
            "simulator.messages.mobility").attr("MobilityCommandType");

    py::object ctl = MobilityCommandTypeL(
            mob_command.attr("command").cast<int>());
    if (ctl.is(MobilityCommandTypeL.attr("SET_MODE"))) {
        command->setCommandType(MobilityCommandType::IDLE_TIME);

    } else if (ctl.is(MobilityCommandTypeL.attr("GOTO_COORDS"))) {
        command->setCommandType(MobilityCommandType::GOTO_COORDS);

    } else if (ctl.is(MobilityCommandTypeL.attr("GOTO_WAYPOINT"))) {
        command->setCommandType(MobilityCommandType::GOTO_WAYPOINT);

    } else if (ctl.is(MobilityCommandTypeL.attr("REVERSE"))) {
        command->setCommandType(MobilityCommandType::REVERSE);

    } else {
        std::cout << "Something is wrong for transformToMobilityCommandPython"
                << std::endl;
    }

    command->setParam1(mob_command.attr("param_1").cast<int>());
    command->setParam2(mob_command.attr("param_2").cast<int>());
    command->setParam3(mob_command.attr("param_3").cast<int>());
    command->setParam4(mob_command.attr("param_4").cast<int>());
    command->setParam5(mob_command.attr("param_5").cast<int>());

    return command;
}

static std::string concatenate(const std::string &str, int num) {
    std::string numStr = std::to_string(num);

    return str + "_for_" + numStr;
}

}

#endif /* gradys_simulations_PYTHON_UTILS_H_ */
