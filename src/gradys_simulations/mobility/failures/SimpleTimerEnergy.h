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

#ifndef MOBILITY_FAILURES_SIMPLETIMERENERGY_H_
#define MOBILITY_FAILURES_SIMPLETIMERENERGY_H_

#include "gradys_simulations/protocols/messages/internal/Telemetry_m.h"
#include <omnetpp.h>

using namespace omnetpp;

namespace gradys_simulations {

class SimpleTimerEnergy : public cSimpleModule {
    public:
        virtual void initialize(int stage) override;

        virtual void handleMessage(cMessage *msg) override;

        virtual ~SimpleTimerEnergy();

    public:
        simtime_t batteryRTLDuration;
        simtime_t batteryShutdownDuration;
        simtime_t rechargeDuration;

    protected:
        Telemetry currentTelemetry;
        cMessage *RTLMessage;
        cMessage *shutdownMessage;
        bool isReturning;
};

} /* namespace gradys_simulations */

#endif /* MOBILITY_FAILURES_SIMPLETIMERENERGY_H_ */
