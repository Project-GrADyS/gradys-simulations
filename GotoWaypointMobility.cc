#include "inet/common/INETDefs.h"
#include "GotoWaypointMobility.h"
#include "stdlib.h"
#include <time.h>
#include <iostream>

using namespace inet;

namespace projeto {

Define_Module(GotoWaypointMobility);

GotoWaypointMobility::GotoWaypointMobility()
{
    nextMoveIsWait = false;

    //std::cout  << " Finished Constructor GotoWaypointMobility() " << endl;

}

void GotoWaypointMobility::setInitialPosition()
{
    // not all mobility models have "initialX", "initialY" and "initialZ" parameters
    lastPosition.x = par("initialXgoTo");
    lastPosition.y = par("initialYgoTo");
    lastPosition.z = par("initialZgoTo");

    EV_DEBUG << "position initialized from initialX/Y/Z parameters: " << lastPosition << endl;

    if (par("updateDisplayString"))
        updateDisplayStringFromMobilityState();

    // std::cout  << "XGH_debug: lastPosition: " << lastPosition << endl;
}

void GotoWaypointMobility::setTargetPosition()
{
    if (nextMoveIsWait) {
        simtime_t waitTime = waitTimeParameter->doubleValue();
        nextChange = simTime() + waitTime;
        nextMoveIsWait = false;
    }
    else {
        //std::cout  << "getParentModule: "  <<  this->getParentModule() << endl;
        auto myParentNode = this->getParentModule();

        auto nextWayPonint = myParentNode->par("wayPointIndex").intValue();

        ////////////
//        int myParentNodeId = myParentNode->getId();
//        auto simulation = getSimulation();
//        auto teste = simulation->getModule(myParentNodeId)->refreshNextWayPoint();
//        std::cout  << "simulation->getModule(myParentNodeId)->getId();: "  <<  teste << endl;

        ///////////

        std::string tmp = std::to_string(nextWayPonint);
        std::string axisX = "nextX_" + tmp;
        std::string axisY = "nextY_" + tmp;
        std::string axisZ = "nextZ_" + tmp;

        auto nextX = myParentNode->par(axisX.c_str()).doubleValue();
        auto nextY = myParentNode->par(axisY.c_str()).doubleValue();
        auto nextZ = myParentNode->par(axisZ.c_str()).doubleValue();

        Coord playTargetPosition;
        playTargetPosition.setX(nextX);
        playTargetPosition.setY(nextY);
        playTargetPosition.setZ(nextZ);

        //std::cout  << "Seting o wayPointIndex " << nextWayPonint << endl;
        int nextWP = ++nextWayPonint % 4;
        myParentNode->par("wayPointIndex").setIntValue(nextWP);

        //std::cout << "playTargetPosition TEXTO DA REUNIAO = " << playTargetPosition.getXyz() << endl;

        targetPosition = playTargetPosition;
        //targetPosition = getRandomPosition();

        double speed = speedParameter->doubleValue();
        double distance = lastPosition.distance(targetPosition);
        simtime_t travelTime = distance / speed;
        nextChange = simTime() + travelTime;
        nextMoveIsWait = hasWaitTime;
    }
}



} // namespace inet

