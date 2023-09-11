/*
 * VehicleTypes.h
 *
 *  Created on: 12 de mar de 2022
 *      Author: thlam
 */

#ifndef gradys_simulations_UTILS_CONSEQUENCE_TYPE_H_
#define gradys_simulations_UTILS_CONSEQUENCE_TYPE_H_

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

static ConsequenceType transformToConsequenceTypePython(std::string type) {
    if (type == "COMMUNICATION") {
        return ConsequenceType::COMMUNICATION;
    } else if (type == "MOBILITY") {
        return ConsequenceType::MOBILITY;
    } else if (type == "TIMER") {
        return ConsequenceType::TIMER;
    } else {
        return ConsequenceType::TRACK_VARIABLE;
    }
}

static CommunicationCommandType transformToCommunicationCommandTypePython(
        std::string type) {
    if (type == "SEND") {
        return CommunicationCommandType::SEND_MESSAGE;
    } else {
        return CommunicationCommandType::SET_TARGET;
    }
}

static CommunicationCommand* transformToCommunicationCommandPython(
        py::object comm_command) {

    py::dict message = comm_command.attr("message");
    nlohmann::json jsonMessage = message;

    PythonMessage *payload = new PythonMessage();
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());

    payload->setMap(jsonMessage);

    // Sends command to the communication module to start using this message
    CommunicationCommand *command = new CommunicationCommand();
    command->setCommandType(
            transformToCommunicationCommandTypePython(
                    comm_command.attr("command").cast<std::string>()));
    command->setPayloadTemplate(payload);

    return command;
}

static MobilityCommandType transformToMobilityCommandTypePython(
        std::string type) {
    if (type == "SET_MODE") {
        return MobilityCommandType::IDLE_TIME;
    } else if (type == "GOTO_COORDS") {
        return MobilityCommandType::GOTO_COORDS;
    } else if (type == "GOTO_WAYPOINT") {
        return MobilityCommandType::GOTO_WAYPOINT;
    } else {
        return MobilityCommandType::REVERSE;
    }
}

static MobilityCommand* transformToMobilityCommandPython(
        py::object mob_command) {
    MobilityCommand *command = new MobilityCommand();

    command->setCommandType(
            transformToMobilityCommandTypePython(
                    mob_command.attr("command").cast<std::string>()));

    command->setParam1(mob_command.attr("param_1").cast<int>());
    command->setParam2(mob_command.attr("param_2").cast<int>());
    command->setParam3(mob_command.attr("param_3").cast<int>());
    command->setParam4(mob_command.attr("param_4").cast<int>());
    command->setParam5(mob_command.attr("param_5").cast<int>());

    return command;
}

static std::string concatenate(const std::string &str, int num) {
    std::string numStr = std::to_string(num);

    return str + "|" + numStr;
}

static void retrieve(const std::string &concatenated, std::string &str,
        int &num) {
    size_t delimiterPos = concatenated.find("|");

    if (delimiterPos != std::string::npos) {
        str = concatenated.substr(0, delimiterPos);
        std::string numStr = concatenated.substr(delimiterPos + 1);

        std::istringstream(numStr) >> num;
    } else {
        str = "";
        num = -1;
    }
}

//enum class CommunicationCommandTypePython {
//    SEND = 1, BROADCAST = 2
//};

//struct CommunicationCommandPython {
//    CommunicationCommandTypePython command;
//    py::dict message;
//};

//enum class MobilityCommandTypePython {
//    SET_MODE = 1, GOTO_COORDS = 2, GOTO_WAYPOINT = 3, REVERSE = 4
//};

//enum class MobilityModePython {
//    GUIDED = 1, AUTO = 2
//};

//struct MobilityCommandPython {
//    MobilityCommandTypePython command;
//    int param_1;
//    int param_2;
//    int param_3;
//    int param_4;
//    int param_5;
//    int param_6;
//};

//MobilityCommandTypePython transformToMobilityCommandTypePython(
//        std::string type) {
//    if (type == "SET_MODE") {
//        return MobilityCommandTypePython::SET_MODE;
//    } else if (type == "GOTO_COORDS") {
//        return MobilityCommandTypePython::GOTO_COORDS;
//    } else if (type == "GOTO_WAYPOINT") {
//        return MobilityCommandTypePython::GOTO_WAYPOINT;
//    } else {
//        return MobilityCommandTypePython::REVERSE;
//    }
//}

//MobilityCommandPython transformToMobilityCommandPython(
//        py::object obj) {
//    MobilityCommandPython ccp;
//
//    ccp.command = transformToMobilityCommandTypePython(
//            obj.attr("command").cast<std::string>());
//
//    ccp.param_1 = obj.attr("param_1").cast<int>();
//    ccp.param_2 = obj.attr("param_2").cast<int>();
//    ccp.param_3 = obj.attr("param_3").cast<int>();
//    ccp.param_4 = obj.attr("param_4").cast<int>();
//    ccp.param_5 = obj.attr("param_5").cast<int>();
//    ccp.param_6 = obj.attr("param_6").cast<int>();
//
//    return ccp;
//}

//MobilityCommandPython transformToMobilityCommandPython(
//        py::object obj) {
//    MobilityCommandPython ccp;
//
//    ccp.command = transformToMobilityCommandTypePython(
//            obj.attr("command").cast<std::string>());
//
//    ccp.param_1 = obj.attr("param_1").cast<int>();
//    ccp.param_2 = obj.attr("param_2").cast<int>();
//    ccp.param_3 = obj.attr("param_3").cast<int>();
//    ccp.param_4 = obj.attr("param_4").cast<int>();
//    ccp.param_5 = obj.attr("param_5").cast<int>();
//    ccp.param_6 = obj.attr("param_6").cast<int>();
//
//    return ccp;
//}

//PYBIND11_EMBEDDED_MODULE(simulatord, sim) {
//
//    py::enum_<ConsequenceTypePython>(sim, "_ConsequenceType")
//    .value("COMMUNICATION", ConsequenceTypePython::COMMUNICATION)
//    .value("MOBILITY", ConsequenceTypePython::MOBILITY)
//    .value("TIMER", ConsequenceTypePython::TIMER)
//    .export_values();
//
//    py::enum_<CommunicationCommandTypePython>(sim, "CommunicationCommandType")
//    .value("SEND", CommunicationCommandTypePython::SEND)
//    .value("BROADCAST", CommunicationCommandTypePython::BROADCAST)
//    .export_values();
//
//    py::class_<CommunicationCommandPython>(sim, "CommunicationCommand")
//    .def(py::init<>())
//    .def_readwrite("command", &CommunicationCommandPython::command)
//    .def_readwrite("message", &CommunicationCommandPython::message);

//    py::enum_<MobilityCommandType>(sim, "MobilityCommandType")
//    .value("SET_MODE", MobilityCommandType::SET_MODE)
//    .value("GOTO_COORDS", MobilityCommandType::GOTO_COORDS)
//    .value("GOTO_WAYPOINT", MobilityCommandType::GOTO_WAYPOINT)
//    .value("REVERSE", MobilityCommandType::REVERSE)
//    .export_values();

//    py::enum_<MobilityModePython>(sim, "MobilityMode")
//    .value("GUIDED", MobilityModePython::GUIDED)
//    .value("AUTO", MobilityModePython::AUTO)
//    .export_values();

//    py::class_<MobilityCommand>(sim, "MobilityCommand")
//    .def_readwrite("command", &MobilityCommandPython::command)
//    .def_readwrite("param_1", &MobilityCommandPython::param_1)
//    .def_readwrite("param_2", &MobilityCommandPython::param_2)
//    .def_readwrite("param_3", &MobilityCommandPython::param_3)
//    .def_readwrite("param_4", &MobilityCommandPython::param_4)
//    .def_readwrite("param_5", &MobilityCommandPython::param_5)
//    .def_readwrite("param_6", &MobilityCommandPython::param_6);

//}

}

#endif /* gradys_simulations_UTILS_CONSEQUENCE_TYPE_H_ */
