#include <omnetpp.h>
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

void MobileNode::handleMessage(cMessage *msg) {

    std::cout  << " MobileNode::handleMessage: " << msg << endl;

}

} //namespace
