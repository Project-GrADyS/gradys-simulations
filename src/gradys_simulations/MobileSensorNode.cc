#include "MobileSensorNode.h"
#include "stdlib.h"
#include <time.h>
#include "MobileSensorNode.h"

using namespace inet;

namespace gradys_simulations {

//Register_Class(MobileNode);
Define_Module(MobileSensorNode);

void MobileSensorNode::initialize(){

    // TO-DO: these ID shaw be UUID as in network
    internalMobNodeId = this->getId() ;//-  par("simulationIndexOfFirstNode").intValue() + 1;
    par("internalMobNodeId").setIntValue(internalMobNodeId);

    //this->myType = static_cast<mobileNodeType>(par("nodeType").intValue());

    std::cout << "Sensor initialization of internalMobNodeId " << internalMobNodeId << " Class " << this->getClassName() << "." << endl;

}
int MobileSensorNode::processMessage(inet::Packet *msg) {

    // GETNAME returns the payload
    std::cout  << "Sensor-" << internalMobNodeId << " received: " << msg->getName() << endl;

    this->shouldSendAMsg = true;

    return 1;

}

string MobileSensorNode::generateNextPacketToSend(){

    std::ostringstream payload;

    payload << "Hi from " << "Sensor-" <<  internalMobNodeId << "{" << ++sentMsgs << "}" << endl;

    return payload.str().c_str();
};


void MobileSensorNode::handleMessage(cMessage *msg) {

    std::cout  << " MobileSensorNode::handleMessage: " << msg << endl;

}

} //namespace
