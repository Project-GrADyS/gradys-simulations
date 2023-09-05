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

#ifndef COMMUNICATION_PROTOCOLS_SENSOR_PYTHONDRONEPROTOCOL_H_
#define COMMUNICATION_PROTOCOLS_SENSOR_PYTHONDRONEPROTOCOL_H_

#include "pybind11/embed.h"

#include <omnetpp.h>

#include "gradys_simulations/utils/Singleton.h"
#include "../base/CommunicationProtocolBase.h"
#include "../messages/network/SimpleMessage_m.h"

namespace py = pybind11;

namespace gradys_simulations {

class PythonSensorProtocol: public CommunicationProtocolBase {
protected:
    virtual void handleTimer(cMessage *msg);

    virtual void handleMessage(cMessage *msg) override;

    virtual void handleTelemetry(gradys_simulations::Telemetry *telemetry) override;

    virtual void initialize(int stage) override;

    virtual void handlePacket(Packet *pk) override;

    virtual void finish() override;

private:
    virtual void dealWithConsequence(py::object consequence);

    CommunicationCommand *communicationCommand;
    MobilityCommand *mobilityCommand;
    cMessage *timer;

    std::map<std::string, std::string> content;

    py::object instance;
    Singleton *pythonInterpreter;
};

} /* namespace gradys_simulations */

#endif /* COMMUNICATION_PROTOCOLS_SENSOR_PYTHONDRONEPROTOCOL_H_ */
