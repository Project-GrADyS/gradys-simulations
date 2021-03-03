#include "MobileNode.h"
#include "stdlib.h"
#include <time.h>
#include "MobileNode.h"


namespace projeto {

Register_Class(MobileNode);
//Define_Module(MobileNode);

void MobileNode::initialize(){



    // TO-DO: these ID shaw be UUID as in network
    internalMobNodeId = this->getId() -  par("simulationIndexOfFirstNode").intValue() + 1;
    par("internalMobNodeId").setIntValue(internalMobNodeId);

    //this->myType = static_cast<mobileNodeType>(par("nodeType").intValue());

    std::cout << "UAV initialization of internalMobNodeId: " << internalMobNodeId << " Class " << this->getClassName() << "." << endl;


    for (int n= 0; n< par("wayPointSetSize").intValue(); n++){

        if (n == 0 ){
            waypoints[n].x = 0;
            waypoints[n].y = 0;
        } else {
            if (n % 2 == 0){
              waypoints[n].x = n * 100 + internalMobNodeId * 200;
              waypoints[n].y = n * 100 + internalMobNodeId * 200;
            } else {
                if (n % 3 == 0) {
                    waypoints[n].x = 2 * 100 + internalMobNodeId * 200;
                    waypoints[n].y = 0 * 100 + internalMobNodeId * 200;
                } else {
                    waypoints[n].x = 0 * 100 + internalMobNodeId * 200;
                    waypoints[n].y = 2 * 100 + internalMobNodeId * 200;

                }
            }
        }
        waypoints[n].z = 10;// + internalMobNodeId;
    }

    par("nextX_0").setDoubleValue(0);
    par("nextY_0").setDoubleValue(0);
    par("nextZ_0").setDoubleValue(10);

    par("nextX_1").setDoubleValue(0 * 100 + internalMobNodeId*100);
    par("nextY_1").setDoubleValue(2 * 100 + internalMobNodeId*100);
    par("nextZ_1").setDoubleValue(10);// + internalMobNodeId);

    par("nextX_2").setDoubleValue(2 * 100 + internalMobNodeId*100);
    par("nextY_2").setDoubleValue(2 * 100 + internalMobNodeId*100);
    par("nextZ_2").setDoubleValue(10);// + internalMobNodeId);

    par("nextX_3").setDoubleValue(2 * 100 + internalMobNodeId*100);
    par("nextY_3").setDoubleValue(0 * 100 + internalMobNodeId*100);
    par("nextZ_3").setDoubleValue(10);// + internalMobNodeId);

}
int MobileNode::processMessage(inet::Packet *msg) {

    // O getname � o payload
    std::cout  << "UAV-" << internalMobNodeId << " received: " << msg->getName() << endl;

    return 1;

}

string MobileNode::generateNextPacketToSend(){

    std::ostringstream payload;

    payload << "Hi from " << "UAV-" <<  internalMobNodeId << "{" << ++sentMsgs << "}" << endl;

    return payload.str().c_str();
};


int MobileNode::refreshNextWayPoint() {

    //int um = 1;
    //std::cout  << "\n\nrefreshNextWayPoint() = " << par("internalMobNodeId").intValue() << endl;
    return internalMobNodeId;

}

void MobileNode::handleMessage(cMessage *msg) {

    std::cout  << " MobileNode::handleMessage: " << msg << endl;

}

} //namespace
