//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <string>
#include <string.h>

#include "gradys_simulations/applications/mamapp/BMeshPacket_m.h"
#include "gradys_simulations/applications/mamapp/MamDataCollectorApp.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"

namespace gradys_simulations {

Define_Module(MamDataCollectorApp);

MamDataCollectorApp::~MamDataCollectorApp()
{
    cancelAndDelete(selfMsg);
}

void MamDataCollectorApp::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        numSentDiscovery = 0;
        WATCH(numSentDiscovery);

        numReceived = 0;
        numUnique = 0;
        numSenders = 0;

        uniqueDataBytesReceived = 0;
        excessDataBytesReceived = 0;
        excessDataPacketsReceived = 0;

        WATCH(numReceived);
        WATCH(numUnique);
        WATCH(numSenders);

        localPort = par("localPort");
        destPort = par("destPort");
        startTime = par("startTime");
        stopTime = par("stopTime");
        dontFragment = par("dontFragment");

        if (stopTime >= SIMTIME_ZERO && stopTime < startTime)
            throw cRuntimeError("Invalid startTime/stopTime parameters");
        selfMsg = new cMessage("UDPSinkTimer");

        dataDelaySignal = registerSignal("dataDelay");
    }
}

void MamDataCollectorApp::handleMessageWhenUp(cMessage *msg)
{
    if (msg->isSelfMessage()) {
        ASSERT(msg == selfMsg);
        switch (selfMsg->getKind()) {
            case START:
                processStart();
                break;

            case STOP:
                processStop();
                break;

            case DISCOVERY:
                processDiscovery();
                break;

            default:
                throw cRuntimeError("Invalid kind %d in self message", (int)selfMsg->getKind());
        }
    }
    else if (msg->arrivedOn("socketIn"))
        socket.processMessage(msg);
    else if (msg->arrivedOn("protocolGate$i")) {

    }
    else
        throw cRuntimeError("Unknown incoming gate: '%s'", msg->getArrivalGate()->getFullName());
}

void MamDataCollectorApp::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    // process incoming packet
    processPacket(packet);
}

void MamDataCollectorApp::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    EV_WARN << "Ignoring UDP error report " << indication->getName() << endl;
    delete indication;
}

void MamDataCollectorApp::socketClosed(UdpSocket *socket)
{
    if (operationalState == State::STOPPING_OPERATION)
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));
}

void MamDataCollectorApp::refreshDisplay() const
{
    ApplicationBase::refreshDisplay();

    char buf[128];
    sprintf(buf, "rcvd: %d pks (%d unique from %d senders)", numReceived, numUnique, numSenders);
    getDisplayString().setTagArg("t", 0, buf);
}

void MamDataCollectorApp::finish()
{
    ApplicationBase::finish();

    recordScalar("unique data packets received", uniqueDataSendPacketHashes.size());
    recordScalar("unique data packets bytes received", uniqueDataBytesReceived);

    recordScalar("repeated data packets received", excessDataPacketsReceived);
    recordScalar("repeated data packets bytes received", excessDataBytesReceived);

    //uuidLength = 36;
    //separatorLength=1 (,)
    //maxKeyLength = 32768;
    //maxUUIDs per part - let's leave it at 750

    std::string uniqueReceivedPacketUUIDsStr = "received packet uuids-part1=";

    int i = 0;
    int page = 1;
    for (auto const& e : uniqueDataSendPacketHashes)
    {
        if (i == 750) {
            uniqueReceivedPacketUUIDsStr.pop_back();
            recordScalar(uniqueReceivedPacketUUIDsStr.c_str(), 1.0);
            i = 0;
            uniqueReceivedPacketUUIDsStr = "received packet uuids-part" + std::to_string(++page) + "=";
        }

        uniqueReceivedPacketUUIDsStr += e;
        uniqueReceivedPacketUUIDsStr += ',';
        i++;
    }

    uniqueReceivedPacketUUIDsStr.pop_back();
    recordScalar(uniqueReceivedPacketUUIDsStr.c_str(), 1.0);



    EV_INFO << getFullPath() << ": received " << numReceived << " packets (" << numUnique <<
            " unique from " << numSenders << " senders)\n";
}

void MamDataCollectorApp::setSocketOptions()
{
    int timeToLive = findPar("timeToLive");
    if (timeToLive != -1)
        socket.setTimeToLive(timeToLive);

    int dscp = findPar("dscp");
    //if (dscp != -1)
    //    socket.setDscp(dscp);

    int tos = findPar("tos");
    if (tos != -1)
        socket.setTos(tos);

    const char *multicastInterface = par("multicastInterface");
    if (multicastInterface[0]) {
        IInterfaceTable *ift = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
        NetworkInterface *ie = ift->findInterfaceByName(multicastInterface);
        if (!ie)
            throw cRuntimeError("Wrong multicastInterface setting: no interface named \"%s\"", multicastInterface);
        socket.setMulticastOutputInterface(ie->getInterfaceId());
    }

    bool receiveBroadcast = findPar("receiveBroadcast");
    if (receiveBroadcast)
        socket.setBroadcast(true);

    bool joinLocalMulticastGroups = findPar("joinLocalMulticastGroups");
    if (joinLocalMulticastGroups) {
        MulticastGroupList mgl = getModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this)->collectMulticastGroups();
        socket.joinLocalMulticastGroups(mgl);
    }
    socket.setCallback(this);
}

void MamDataCollectorApp::processStart()
{
    socket.setOutputGate(gate("socketOut"));
    socket.bind(localPort);
    setSocketOptions();

    if (stopTime >= SIMTIME_ZERO) {
        selfMsg->setKind(STOP);
        scheduleAt(stopTime, selfMsg);
    }
    else {
        selfMsg->setKind(DISCOVERY);
        scheduleAt(simTime() + par("sendDiscoveryInterval"), selfMsg);
    }
}

void MamDataCollectorApp::processDiscovery()
{
    sendDiscoveryPacket();
    simtime_t d = simTime() + par("sendDiscoveryInterval");
    if (stopTime < SIMTIME_ZERO || d < stopTime) {
        selfMsg->setKind(DISCOVERY);
        scheduleAt(d, selfMsg);
    }
    else {
        selfMsg->setKind(STOP);
        scheduleAt(stopTime, selfMsg);
    }
}

void MamDataCollectorApp::sendDiscoveryPacket()
{
    std::ostringstream str;
    str << "MAMCDISCOVERY";
    Packet *packet = new Packet(str.str().c_str());

    if (dontFragment)
        packet->addTagIfAbsent<FragmentationReq>()->setDontFragment(true);

    const auto& payload = makeShared<BMeshPacket>();
    payload->setChunkLength(B(1));
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    packet->insertAtBack(payload);
    L3Address broadcastAddr;
    broadcastAddr.set(Ipv4Address(0xFFFFFFFF));

    //char addr[1024] = "host1";
    //L3AddressResolver().tryResolve(addr, broadcastAddr);

    emit(packetSentSignal, packet);
    socket.sendTo(packet, broadcastAddr, destPort);
    numSentDiscovery++;
}

void MamDataCollectorApp::sendDataAckPacket()
{
    std::ostringstream str;
    str << "DATA-ACK";
    Packet *packet = new Packet(str.str().c_str());

    if (dontFragment)
        packet->addTagIfAbsent<FragmentationReq>()->setDontFragment(true);

    const auto& payload = makeShared<BMeshPacket>();
    payload->setChunkLength(B(1));
    //payload->setSrcUuid(srcUuid)
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    packet->insertAtBack(payload);
    L3Address broadcastAddr;
    broadcastAddr.set(Ipv4Address(0xFFFFFFFF));

    //char addr[1024] = "host1";
    //L3AddressResolver().tryResolve(addr, broadcastAddr);

    emit(packetSentSignal, packet);
    socket.sendTo(packet, broadcastAddr, destPort);
    numSentDataAck++;
}

void MamDataCollectorApp::processStop()
{
    if (!multicastGroup.isUnspecified())
        socket.leaveMulticastGroup(multicastGroup); // FIXME should be done by socket.close()
    socket.close();
}

void MamDataCollectorApp::processPacket(Packet *pk)
{
    auto sizeBytes = pk->getByteLength();
    EV_INFO << "Received packet: " << UdpSocket::getReceivedPacketInfo(pk) << " size = " << sizeBytes << " bytes" << endl;
    emit(packetReceivedSignal, pk);

    auto bmeshData = dynamicPtrCast<const BMeshPacket>(pk->peekAtBack(B(1), 1));

    if (bmeshData != nullptr) {

        int sequence = bmeshData->getSequence();
        std::string packetId = bmeshData->getPacketUuid();

        if (sequence > 0) {
            //auto createdAtTag = bmeshData->getTag<CreationTimeTag>();
            auto creationTime = bmeshData->getCreationTime();

            ASSERT(sizeBytes == 11);
            //ASSERT(createdAtTag->getCreationTime() == creationTime); THIS IS NOT HOLDING. So we won't use the tag data for now.

            auto delay = simTime() - creationTime; // compute delay

            emit(dataDelaySignal, delay);

            if (uniqueDataSendPacketHashes.find(bmeshData->getPacketUuid()) == uniqueDataSendPacketHashes.end()) {
                uniqueDataSendPacketHashes.insert(bmeshData->getPacketUuid());
                uniqueDataBytesReceived += sizeBytes;

                // Sends unique packet to protocol module
                cGate *protocolGate = gate("protocolGate$o");
                if(protocolGate->isConnected()) {
                    send(pk->dup(), protocolGate);
                }
            } else {
                excessDataPacketsReceived++;
                excessDataBytesReceived += sizeBytes;
            }

            uniqueDataSenders.insert(bmeshData->getSrcUuid());

            numUnique = uniqueDataSendPacketHashes.size();
            numSenders = uniqueDataSenders.size();

            numReceived++;

            //sendAck(bmeshData->getPacketUuid());
        }
    }

    delete pk;
}

void MamDataCollectorApp::handleStartOperation(LifecycleOperation *operation)
{
    simtime_t start = std::max(startTime, simTime());
    if ((stopTime < SIMTIME_ZERO) || (start < stopTime) || (start == stopTime && startTime == stopTime)) {
        selfMsg->setKind(START);
        scheduleAt(start, selfMsg);
    }
}

void MamDataCollectorApp::handleStopOperation(LifecycleOperation *operation)
{
    cancelEvent(selfMsg);
    if (!multicastGroup.isUnspecified())
        socket.leaveMulticastGroup(multicastGroup); // FIXME should be done by socket.close()
    socket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void MamDataCollectorApp::handleCrashOperation(LifecycleOperation *operation)
{
    cancelEvent(selfMsg);
    if (operation->getRootModule() != getContainingNode(this)) {     // closes socket when the application crashed only
        if (!multicastGroup.isUnspecified())
            socket.leaveMulticastGroup(multicastGroup); // FIXME should be done by socket.close()
        socket.destroy();    //TODO  in real operating systems, program crash detected by OS and OS closes sockets of crashed programs.
    }
}

} // namespace inet

