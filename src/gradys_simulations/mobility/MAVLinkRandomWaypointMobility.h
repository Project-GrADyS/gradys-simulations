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

#ifndef __gradys_simulations_MAVLINKRANDOMWAYPOINTMOBILITY_H_
#define __gradys_simulations_MAVLINKRANDOMWAYPOINTMOBILITY_H_

#include <omnetpp.h>
#include "gradys_simulations/mobility/base/MAVLinkMobilityBase.h"
#include "inet/mobility/single/RandomWaypointMobility.h"

namespace gradys_simulations {

/**
 * This class doesn't extend RandomWaypointMobility because it would cause conflicts with the
 * necessary MAVLinkMobilityBase base class. It implements the same NED parameters and it's
 * usage should be more or less identical, other than the added setup of MAVLinkMobilityBase.
 */
class MAVLinkRandomWaypointMobility : public MAVLinkMobilityBase
{
protected:
    double speed;
    simtime_t waitTime;
    double waypointRadius;
    omnetpp::cMessage *waypointChangeMessage = new cMessage("waypointChangeMessage");
    Coord targetPosition = Coord::NIL;
protected:
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    /**
     * Function used to set the next waypoint to move to.
     * It generates points randonly distributed in the constrain space of the mobility module.
     */
    virtual void setTargetPosition();
    /**
     * Sets the correct flying mode and parameters. Finally it instructs the drone to takeoff
     */
    virtual void startMovement();

    virtual void move() override;
};

} //namespace

#endif
