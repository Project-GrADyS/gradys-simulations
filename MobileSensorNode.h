#include "inet/common/INETDefs.h"

#ifndef __MOBILESENSORNODE_H_
#define __MOBILESENSORNODE_H_

#include <omnetpp.h>
//#include <cModule.h>
#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "inet/applications/base/ApplicationBase.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include "inet/applications/base/ApplicationBase.h"
#include "inet/transportlayer/contract/udp/UdpSocket.h"
#include <iostream>
#include <algorithm>
#include <iterator>

using std::cout; using std::endl;
using std::string; using std::reverse;


using namespace omnetpp;
using namespace inet;

namespace projeto {

class WayPoint {
  public:
    double x;
    double y;
    double z;
};

//enum mobileNodeType { sensor = 1, uav = 2, baseStation = 3, missing = 171 };

class MobileSensorNode : public cModule  {
  protected:
    virtual void initialize() override;//(int stage) override;
    virtual void handleMessage(cMessage *msg);
  public:
    int processMessage(inet::Packet *msg);
    string generateNextPacketToSend();
  //  WayPoint waypoints[4];
    int internalMobNodeId;
    long sentMsgs = -1;
    bool shouldSendAMsg = false;


  //  mobileNodeType myType = missing;

};

} //namespace
#endif
