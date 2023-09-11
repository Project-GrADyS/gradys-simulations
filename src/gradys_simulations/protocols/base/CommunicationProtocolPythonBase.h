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

#ifndef __gradys_simulations_COMMUNICATIONPROTOCOLPYTHONBASE_H_
#define __gradys_simulations_COMMUNICATIONPROTOCOLPYTHONBASE_H_

#include <omnetpp.h>
#include "inet/common/packet/Packet.h"
#include "pybind11/embed.h"

#include "gradys_simulations/protocols/messages/internal/MobilityCommand_m.h"
#include "gradys_simulations/protocols/messages/internal/CommunicationCommand_m.h"
#include "gradys_simulations/protocols/messages/internal/Telemetry_m.h"
#include "gradys_simulations/utils/Singleton.h"

#include "gradys_simulations/protocols/base/CommunicationProtocolBase.h"

using namespace omnetpp;
using namespace inet;

namespace py = pybind11;

namespace gradys_simulations {


class CommunicationProtocolPythonBase: public CommunicationProtocolBase {
protected:
    virtual void initialize(int stage, std::string figure);

    virtual void handleMessage(cMessage *msg) override;

    virtual void handlePacket(Packet *pk) override;

    virtual void handleTimer(cMessage *msg);

    virtual void handleTelemetry(Telemetry *telemetry) override;

    virtual void finish() override;

    py::object instance;

    virtual void dealWithConsequence(py::object consequence);

private:
    std::string classType;
    std::map<std::string, std::string> content;

    Singleton *pythonInterpreter;

    virtual void updateGUI();

};

} //namespace

#endif
