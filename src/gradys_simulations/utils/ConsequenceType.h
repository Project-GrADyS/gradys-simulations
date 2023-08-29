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

using namespace pybind11::literals;

namespace py = pybind11;

namespace gradys_simulations {

enum class ConsequenceType {
    COMMUNICATION = 1, MOBILITY = 2, TIMER = 3
};

static constexpr unsigned int str2int(const char *str, int h = 0) {
    return !str[h] ? 5381 : (str2int(str, h + 1) * 33) ^ str[h];
}

static ConsequenceType transformToConsequenceTypePython(std::string type) {
    if (type == "COMMUNICATION") {
        return ConsequenceType::COMMUNICATION;
    } else if (type == "MOBILITY") {
        return ConsequenceType::MOBILITY;
    } else {
        return ConsequenceType::TIMER;
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

    std::string message_sender =
            comm_command.attr("message").attr("sender").cast<std::string>();
    int message_content =
            comm_command.attr("message").attr("content").cast<int>();

    SimpleMessage *payload = new SimpleMessage();
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());

    if (message_sender == "DRONE") {
        payload->setSenderType(SenderType::DRONE);
    } else if (message_sender == "SENSOR") {
        payload->setSenderType(SenderType::SENSOR);
    } else {
        payload->setSenderType(SenderType::GROUND_STATION);
    }

    payload->setContent(message_content);

    // Sends command to the communication module to start using this message
    CommunicationCommand *command = new CommunicationCommand();
    command->setCommandType(
            transformToCommunicationCommandTypePython(
                    comm_command.attr("command").cast<std::string>()));
    command->setPayloadTemplate(payload);

    return command;
}

static py::object getSenderType(int type) {
    py::object SenderType = py::module_::import(
            "simulator.protocols.simple.SimpleMessage").attr("SenderType");

    switch (type) {
    case 0:
        return SenderType.attr("DRONE");
        break;
    case 1:
        return SenderType.attr("SENSOR");
        break;
    default:
        return SenderType.attr("GROUND_STATION");
    }
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
