#include "inet/common/INETDefs.h"


#ifndef __MOBILENODE_H_
#define __MOBILENODE_H_

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

namespace gradys_simulations {

//enum mobileNodeType { sensor = 1, uav = 2, baseStation = 3, missing = 171 };

class MobileNode : public cModule  {
  protected:
    virtual void initialize() override;

};

} //namespace
#endif
