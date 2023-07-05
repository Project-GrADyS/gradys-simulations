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

#ifndef __PROJETO_SIMPLECONSUMPTIONENERGY_H_
#define __PROJETO_SIMPLECONSUMPTIONENERGY_H_

#include <omnetpp.h>
#include "../../protocols/messages/internal/Telemetry_m.h"

using namespace omnetpp;

namespace projeto {

class SimpleConsumptionEnergy : public cSimpleModule
{
  protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

  protected:
    double batteryCapacity;
    double batteryRTLThreshold;
    double batteryConsumption;
    simtime_t rechargeDuration;
  private:
    double currentConsumption;
    Telemetry currentTelemetry;
    cMessage *selfMessage;
    bool isReturning;
};

} //namespace

#endif
