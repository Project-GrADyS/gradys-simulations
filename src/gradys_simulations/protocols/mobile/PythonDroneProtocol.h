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

#ifndef COMMUNICATION_PROTOCOLS_MOBILE_PYTHONDRONEPROTOCOL_H_
#define COMMUNICATION_PROTOCOLS_MOBILE_PYTHONDRONEPROTOCOL_H_

#include <omnetpp.h>

#include "gradys_simulations/protocols/base/CommunicationProtocolPythonBase.h"
#include "gradys_simulations/utils/Singleton.h"

namespace gradys_simulations {

class PythonDroneProtocol: public CommunicationProtocolPythonBase {

protected:
    std::string customProtocolLocation;
    std::string protocol;
    std::string protocolFileName;
    std::string protocolType;

    virtual void initialize(int stage) override;
};

} /* namespace gradys_simulations */

#endif /* COMMUNICATION_PROTOCOLS_MOBILE_PYTHONDRONEPROTOCOL_H_ */
