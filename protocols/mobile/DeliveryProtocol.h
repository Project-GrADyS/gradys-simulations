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

#ifndef COMMUNICATION_PROTOCOLS_MOBILE_DeliveryProtocol_H_
#define COMMUNICATION_PROTOCOLS_MOBILE_DeliveryProtocol_H_

#include <omnetpp.h>
#include "../base/CommunicationProtocolBase.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/geometry/common/GeographicCoordinateSystem.h"

static int agentIndex = 0;

namespace projeto {
class DeliveryProtocol: public CommunicationProtocolBase {
    protected:
        simtime_t timeoutDuration;


        int index = agentIndex++;


        std::vector<GeoCoord> targets = {
                GeoCoord(deg(40.753324), deg(-73.998672), m(10)),
                GeoCoord(deg(40.753324), deg(-73.997643), m(10)),
                GeoCoord(deg(40.753324), deg(-73.996615), m(10)),
                GeoCoord(deg(40.753324), deg(-73.995586), m(10)),
                GeoCoord(deg(40.753324), deg(-73.994558), m(10)),
                GeoCoord(deg(40.752328), deg(-73.998672), m(10)),
                GeoCoord(deg(40.752328), deg(-73.997643), m(10)),
                GeoCoord(deg(40.752328), deg(-73.996615), m(10)),
                GeoCoord(deg(40.752328), deg(-73.995586), m(10)),
                GeoCoord(deg(40.752328), deg(-73.994558), m(10)),
                GeoCoord(deg(40.751333), deg(-73.998672), m(10)),
                GeoCoord(deg(40.751333), deg(-73.997643), m(10)),
                GeoCoord(deg(40.751333), deg(-73.996615), m(10)),
                GeoCoord(deg(40.751333), deg(-73.995586), m(10)),
                GeoCoord(deg(40.751333), deg(-73.994558), m(10)),
                GeoCoord(deg(40.750337), deg(-73.998672), m(10)),
                GeoCoord(deg(40.750337), deg(-73.997643), m(10)),
                GeoCoord(deg(40.750337), deg(-73.996615), m(10)),
                GeoCoord(deg(40.750337), deg(-73.995586), m(10)),
                GeoCoord(deg(40.750337), deg(-73.994558), m(10)),
                GeoCoord(deg(40.749342), deg(-73.998672), m(10)),
                GeoCoord(deg(40.749342), deg(-73.997643), m(10)),
                GeoCoord(deg(40.749342), deg(-73.996615), m(10)),
                GeoCoord(deg(40.749342), deg(-73.995586), m(10)),
                GeoCoord(deg(40.749342), deg(-73.994558), m(10))
        };
        int target = std::rand() % targets.size();
        bool hasVisitedTarget = 0;

        int round = 0;
        bool hasWaited = false;
        bool isTraveling = false;
        std::list<int> received = {};

        cMessage *timer = new cMessage("timer");
        cMessage *timeout = new cMessage("timeout");
    protected:
        // Performs the initialization of our module. This is a function that most OMNeT++ modules will override
        virtual void initialize(int stage) override;

        virtual void handleMessage(cMessage *message) override;

        virtual void goToTarget(int target);

        // Called when the simulation finishes
        virtual void finish() override;
        // Gets called when a packet is recieved from the communication module
        virtual void handlePacket(Packet *pk) override;

        virtual void handleTelemetry(Telemetry *telemetry) override;

    private:
};

} /* namespace projeto */

#endif /* COMMUNICATION_PROTOCOLS_MOBILE_SIMPLEDRONEPROTOCOL_H_ */
