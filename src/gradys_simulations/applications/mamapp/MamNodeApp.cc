//
// Copyright (C) 2000 Institut fuer Telematik, Universitaet Karlsruhe
// Copyright (C) 2004,2011 Andras Varga
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

#include <string.h>
#include <string>
#include <random>
#include <sstream>

#include "gradys_simulations/applications/mamapp/MamNodeApp.h"
#include "inet/common/ModuleAccess.h"
#include "inet/common/TagBase_m.h"
#include "inet/common/TimeTag_m.h"
#include "inet/common/lifecycle/ModuleOperations.h"
#include "inet/common/packet/Packet.h"
#include "inet/networklayer/common/FragmentationTag_m.h"
#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/networklayer/common/L3AddressTag_m.h"
#include "inet/transportlayer/contract/udp/UdpControlInfo_m.h"
#include "gradys_simulations/applications/mamapp/Md5.h"

namespace gradys_simulations {

Define_Module(MamNodeApp);

MamNodeApp::~MamNodeApp()
{
    cancelAndDelete(selfMsg);
    cancelAndDelete(pollTimer);
}

void MamNodeApp::initialize(int stage)
{
    ApplicationBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        numSent = 0;
        numReceived = 0;
        numDataSent = 0;
        numDataResent = 0;
        numDataAckReceived = 0;
        WATCH(numSent);
        WATCH(numReceived);
        WATCH(numDataSent);
        WATCH(numDataResent);
        WATCH(numDataAckReceived);

        L3Address empty;
        mobileSink = empty;

        scheduledSendData = false;

        localPort = par("localPort");
        destPort = par("destPort");
        startTime = par("startTime");
        stopTime = par("stopTime");
        packetName = par("packetName");
        dontFragment = par("dontFragment");

        relayNode = par("relayNode");

        const char *relayMode = par("relayMode");
        if (strcmp(relayMode, "BMesh") == 0) {
            mamRelay = false;
        }
        else if (strcmp(relayMode, "MAM") == 0) {
            mamRelay = true;
        }
        else {
            throw cRuntimeError ("Invalid relay mode: \"%s\"", relayMode);
        }

        delta = par("delta");

        lowPowerNode = par("lowPowerNode");
        friendNode = par("friendNode");

        connectedFriendNode = empty;
        friendshipEstablished = false;

        lowPowerNodes.clear();

        dataSendSequence = 0;

        uniqueDataSenders.clear();

        nodeUuid = getFullPath() + generate_hex(32);
        WATCH(nodeUuid);

        if (stopTime >= SIMTIME_ZERO && stopTime < startTime)
            throw cRuntimeError("Invalid startTime/stopTime parameters");
        selfMsg = new cMessage("sendTimer");
        pollTimer = new cMessage("pollTimer");
    }
}

void MamNodeApp::finish()
{
    ApplicationBase::finish();

    recordScalar("packets sent", numSent);
    recordScalar("packets received", numReceived);
    recordScalar("data packets sent", numDataSent);
    recordScalar("data packets resent", numDataResent);
    recordScalar("ack packets received", numDataAckReceived);
    recordScalar("my data sent", uniqueSentPacketUUIDs.size());


    /////

    std::string uniqueSentPacketUUIDsStr = "generated packet uuids-part1=";

    int i = 0;
    int page = 1;
    for (auto const& e : uniqueSentPacketUUIDs)
    {
        if (i == 750) {
            uniqueSentPacketUUIDsStr.pop_back();
            recordScalar(uniqueSentPacketUUIDsStr.c_str(), 1.0);
            i = 0;
            uniqueSentPacketUUIDsStr = "generated packet uuids-part" + std::to_string(++page) + "=";
        }

        uniqueSentPacketUUIDsStr += e;
        uniqueSentPacketUUIDsStr += ',';
        i++;
    }

    uniqueSentPacketUUIDsStr.pop_back();
    recordScalar(uniqueSentPacketUUIDsStr.c_str(), 1.0);

    EV_INFO << getFullPath() << ": sent " << numDataSent << " data packets\n";

    EV_INFO << getFullPath() << ": resent " << numDataResent << " data packets\n";

    EV_INFO << getFullPath() << ": received " << numDataAckReceived << " data ack packets\n";

    EV_INFO << getFullPath() << ": failed to send " << (numDataSent + numDataResent) - numDataAckReceived << " data packets\n";

    EV_INFO << getFullPath() << ": received from " << uniqueDataSenders.size() << " senders)\n";
}

void MamNodeApp::setSocketOptions()
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

L3Address MamNodeApp::chooseDestAddr()
{
    int k = intrand(destAddresses.size());
    if (destAddresses[k].isUnspecified() || destAddresses[k].isLinkLocal()) {
        L3AddressResolver().tryResolve(destAddressStr[k].c_str(), destAddresses[k]);
    }
    return destAddresses[k];
}

void MamNodeApp::processStart()
{
    socket.setOutputGate(gate("socketOut"));
    const char *localAddress = par("localAddress");
    socket.bind(*localAddress ? L3AddressResolver().resolve(localAddress) : L3Address(), localPort);
    setSocketOptions();

    const char *destAddrs = par("destAddresses");
    cStringTokenizer tokenizer(destAddrs);
    const char *token;

    while ((token = tokenizer.nextToken()) != nullptr) {
        destAddressStr.push_back(token);
        L3Address result;
        L3AddressResolver().tryResolve(token, result);
        if (result.isUnspecified())
            EV_ERROR << "cannot resolve destination address: " << token << endl;
        destAddresses.push_back(result);
    }

    if (!destAddresses.empty()) {
        selfMsg->setKind(SEND_MY_DATA);
        // All nodes sending data every second:
        //simtime_t sendTime = (simTime() + SimTime(1000, SIMTIME_MS)).trunc(SIMTIME_MS); // Schedule send to next second
        //scheduleAt(sendTime, selfMsg);
    }
    else {
        EV_ERROR << "ERROR ERROR ERROR:  Should not be here" << endl;

        if (stopTime >= SIMTIME_ZERO) {
            selfMsg->setKind(STOP);
            scheduleAt(stopTime, selfMsg);
        }
    }

    if (lowPowerNode) {
        L3Address empty;
        assert(connectedFriendNode == empty);
        sendFriendRequest();
    }
}

void MamNodeApp::processSendMyData()
{
    scheduledSendData = false;

    L3Address empty;
    if (!mamRelay || mobileSink != empty) {
        L3Address addr;

        if (mamRelay) {
            // Mobile sink address - MAM Bluetooth Mesh customized relay feature
            addr = mobileSink;
        }
        else {
            // Broadcast address - Bluetooth Mesh regular relay feature
            addr.set(Ipv4Address(0xFFFFFFFF));
        }

        const auto& payload = makeShared<BMeshPacket>();
        payload->setChunkLength(B(11)); // 11 bytes (3 bytes opcode + 8 bytes of data)
        payload->setHops(127);

        std::string uuidStr = generate_uuid_v4();

        uniqueSentPacketUUIDs.insert(uuidStr);

        payload->setPacketUuid(uuidStr.c_str());
        payload->setSrcUuid(nodeUuid.c_str());
        payload->setSequence(++dataSendSequence);
        payload->addTag<CreationTimeTag>()->setCreationTime(simTime());

        std::ostringstream str;
        str << DATA_SEND;
        Packet *packet = new Packet(str.str().c_str());

        if (dontFragment)
            packet->addTagIfAbsent<FragmentationReq>()->setDontFragment(true);

        packet->insertAtBack(payload);

        emit(packetSentSignal, packet);
        socket.sendTo(packet, addr, destPort);

        lastSensorDataSent = simTime();
    } else {
        EV_ERROR << "MOBILE SINK NOT FOUND WHEN TRYING TO SEND DATA";
    }

    // All nodes sending data every second:
    //simtime_t sendTime = (simTime() + SimTime(1000, SIMTIME_MS)).trunc(SIMTIME_MS); // Schedule send to next second
    //scheduleAt(sendTime, selfMsg);
}

void MamNodeApp::processStop()
{
    socket.close();
}

void MamNodeApp::handleMessageWhenUp(cMessage *msg)
{
    if (msg == pollTimer) {
        sendFriendPoll();
        simtime_t timeout = (simTime() + SimTime(pollIntervalMs, SIMTIME_MS)).trunc(SIMTIME_MS);
        scheduleAt(timeout, pollTimer);
    }
    else if (msg->isSelfMessage()) {
        ASSERT(msg == selfMsg);
        switch (selfMsg->getKind()) {
            case START:
                processStart();
                break;

            case SEND_MY_DATA:
                processSendMyData();
                break;

            case STOP:
                processStop();
                break;

            default:
                throw cRuntimeError("Invalid kind %d in self message", (int)selfMsg->getKind());
        }
    }
    else {
        auto packet = check_and_cast<Packet *>(msg);

        auto l3Addresses = packet->getTag<L3AddressInd>();
        L3Address srcAddr = l3Addresses->getSrcAddress();

        if (srcAddr.str() == "127.0.0.1") {
            delete msg;
            return;
        }

        if (getEnvir()->isLoggingEnabled()) {
            EV_DEBUG << "Sensor " << this->getParentModule()->getFullName() << " received " << msg->getName() <<
                    " from " << L3AddressResolver().findHostWithAddress(srcAddr)->getFullName() << endl;
        }

        if (strcmp(msg->getName(), FOUND_MOBILE_SINK) == 0) {
            auto bmeshData = dynamicPtrCast<const BMeshPacket>(packet->peekAtBack(B(1), 1));

            processFoundMobileSink(srcAddr, bmeshData->getHops());
            delete msg;
        }
        else if (strcmp(msg->getName(), DISCONNECTED_MOBILE_SINK) == 0) {
            // If the src is the mobile sink currently connected to
            if (srcAddr == mobileSink) {
                L3Address empty;
                mobileSink = empty;
            }

            if (relayNode) {
                broadcastSimpleMessage (DISCONNECTED_MOBILE_SINK);
            }

            if (lowPowerNodes.size() > 0) {
                for (auto itr = lowPowerNodes.begin(); itr != lowPowerNodes.end(); itr++) {
                    Packet dummy; // TODO Identify disconnection
                    itr->second.push(dummy);
                }
            }

            delete msg;
        }
        else if (strcmp(msg->getName(), MAMCDISCOVERY) == 0) {
            processDiscovery(srcAddr);
            delete msg;
        }
        else if (strcmp(msg->getName(), DATA_SEND) == 0) {
            processDataSend(packet, srcAddr);
            // Do not delete msg as it'll be forwarded somewhere. Only the sink should delete it?
        }
        else if (strcmp(msg->getName(), FRIEND_REQUEST) == 0) {
            processFriendRequest(srcAddr);
            delete msg;
        }
        else if (strcmp(msg->getName(), FRIEND_OFFER) == 0) {
            processFriendOffer(srcAddr);
            delete msg;
        }
        else if (strcmp(msg->getName(), FRIEND_POLL) == 0) {
            processFriendPoll(srcAddr);
            delete msg;
        }
        else if (strcmp(msg->getName(), FRIEND_UPDATE) == 0) {
            processFriendUpdate(srcAddr, 0);
            delete msg;
        }
        else {
            socket.processMessage(msg);
        }
    }
}

void MamNodeApp::processDiscovery(L3Address &src) {
    // Discovery messages are from the sink, so it is the optimal route for this node
    mobileSink = L3Address(src);
    sinkHops = 128;
    sinkBestRouteExpiry = simTime() + simtime_t(20000, SIMTIME_MS);

    if (relayNode) {
        broadcastSimpleMessage (FOUND_MOBILE_SINK);
    }

    if (lowPowerNodes.size() > 0) {
        for (auto itr = lowPowerNodes.begin(); itr != lowPowerNodes.end(); itr++) {
            Packet dummy;
            itr->second.push(dummy);
        }
    }
}

void MamNodeApp::sendMyDataToSink() {
    if (scheduledSendData) {
        // Data send is already scheduled. Skipping.
        return;
    }

    L3Address empty;
    // We should schedule the message to be sent if there is a known route to it (mobileSink != empty)
    // or if we are not using mam custom relay (but using Blueooth Mesh's default relay behavior)
    if (mobileSink != empty || !mamRelay) {

        simtime_t start = std::max(lastSensorDataSent + simtime_t(100, SIMTIME_MS), simTime() + simtime_t(1, SIMTIME_MS));

        scheduledSendData = true;

        cancelEvent(selfMsg);
        selfMsg->setKind(SEND_MY_DATA);
        scheduleAt(start, selfMsg);
    }
}

void MamNodeApp::processFoundMobileSink(L3Address &src, int hops) {
    // Mam Relay behavior to set the route to sink
    if (mamRelay) {
        L3Address empty;

        // If sink not set (or expired) or the incoming route sink is closer
        if (mobileSink == empty || simTime() > sinkBestRouteExpiry || hops > sinkHops) {
            mobileSink = L3Address(src);
            sinkHops = hops;
            sinkBestRouteExpiry = simTime() + simtime_t(delta, SIMTIME_MS);
        }
    }

    sendMyDataToSink();

    if (lowPowerNodes.size() > 0) {
        assert(friendNode);
        for (auto itr = lowPowerNodes.begin(); itr != lowPowerNodes.end(); itr++) {
            Packet dummy;
            itr->second.push(dummy);
        }
    }

    if (relayNode) {
        if (mamRelay) {
            // Check last time sent found mobile sink broadcast and only send it if > 100ms
            if (simTime().inUnit(SIMTIME_MS) - lastFoundSinkSent.inUnit(SIMTIME_MS) > 100) {
                lastFoundSinkSent = simTime();
                broadcastSimpleMessage (FOUND_MOBILE_SINK);
            }
        }
        else {
            if (hops > 0) {
                // Bluetooth Mesh relay should relay the message without a timeout but decrease its ttl
                // use cache
                std::string key = md5("FOUND_MOBILE_SINK_" + src.str());
                double msd = simTime().dbl() * 1000;
                long ms = static_cast<long>(msd);
                bool inCache = dataSendCache.exists(key, ms);

                if (!inCache) {
                    dataSendCache.put(key, 1, ms + 1000); // Expire in 1 second
                    broadcastSimpleMessage(FOUND_MOBILE_SINK, hops--);
                }
            }
        }
    }
}

void MamNodeApp::processDataSend(Packet *packet, L3Address &src) {
    // Only relay nodes are interested in Data Send messages
    if (!relayNode) {
        delete packet;
        return;
    }

    L3Address empty;
    if (mamRelay && mobileSink == empty) {
        sendSimpleMessage(DISCONNECTED_MOBILE_SINK, src, 127);
        delete packet;
        return;
    }

    // We'll try to break loops for cases where awaiting for heartbeats
    // from the mobile sink have not timed out yet by using a simple cache with TTL for the packet destination + id
    // If it's cached, drop the packet. Still need to figure out the implications of doing this..... TODO

    auto bmeshData = dynamicPtrCast<const BMeshPacket>(packet->popAtBack(B(1), 1));
    delete packet;

    if (bmeshData != nullptr) {
        auto key = bmeshData->getPacketUuid();
        double msd = simTime().dbl() * 1000;
        long ms = static_cast<long>(msd);
        bool inCache = dataSendCache.exists(key, ms);

        if (!inCache) {
            dataSendCache.put(key, 1, ms + 1000); // Expire in 1 second

            uniqueDataSenders.insert(bmeshData->getSrcUuid());

            if (mamRelay) {
                sendData(bmeshData, mobileSink); // DATA_SEND
            }
            else {
                L3Address broadcastAddr;
                broadcastAddr.set(Ipv4Address(0xFFFFFFFF));
                sendData(bmeshData, broadcastAddr); // DATA_SEND
            }
        }
    }
}

void MamNodeApp::processFriendRequest(L3Address &src) {
    if (friendNode) {
        sendSimpleMessage(FRIEND_OFFER, src, 127);
    }
}
void MamNodeApp::processFriendOffer(L3Address &src) {
    if (!lowPowerNode) {
        throw cRuntimeError("Error: Non-LPN received a friend offer");
    }

    L3Address empty;
    if (connectedFriendNode == empty) {
        connectedFriendNode = src;
        sendFriendPoll();
    }
}

void MamNodeApp::processFriendPoll(L3Address &src) {
    assert(friendNode);

    string key = src.str();
    if (lowPowerNodes.find(key) == lowPowerNodes.end()) {
        queue<Packet> emptyQueue;
        lowPowerNodes[key] = emptyQueue;

        // Send friend update (Conclude friendship establishment on FN side)
        sendSimpleMessage("FRIEND_UPDATE", src, 1);
    }
    else {
        auto q = lowPowerNodes[key];
        if (!q.empty()){
            Packet sendPacket = q.front();

            sendSimpleMessage("FRIEND_UPDATE", src, 1); // Todo add serialized packet data

            q.pop();
        }
    }
    //if (src not in myConnectedLPNs) myConnectedLPNs.put(src);
    //if(pendingMessages.get(src).length > 0) sendFriendUpdate(pendingMessages.get(src).pop());
}

void MamNodeApp::processFriendUpdate(L3Address &src, int moreData) {
    assert(lowPowerNode);
    assert(connectedFriendNode == src);

    if (!friendshipEstablished) {
        friendshipEstablished = true;
        sendFriendEstablishedInternalMessage();
        simtime_t timeout = (simTime() + SimTime(pollIntervalMs, SIMTIME_MS)).trunc(SIMTIME_MS);
        scheduleAt(timeout, pollTimer);
        sendFriendPoll(); // Send a poll now to complete friendship establishment
    }
    else {
        // TODO deserialize message, call handleLowerMessage equivalent (switch between message type
        // Assuming that every friend update is a process found mobile sink
        processFoundMobileSink(src, 20);

        // TODO check if has more messages, if so call sendFriendPoll();
    }

}

void MamNodeApp::processFriendClear(L3Address &src) {
    //myConnectedLPNs.clear(src);
    //pendingMessages.clear(src);
}

void MamNodeApp::sendDataSentAck(Packet *packet, L3Address &dest) {
    std::ostringstream str;
    str << "ACK-" << packet->getId();

    sendSimpleMessage(str.str().c_str(), dest, 127);
}

void MamNodeApp::broadcastSimpleMessage(const char *msg)
{
    broadcastSimpleMessage(msg, 127);
}

void MamNodeApp::broadcastSimpleMessage(const char *msg, int hops)
{
    if (hops < 0) {
        throw cRuntimeError ("Invalid number of hops: %d", hops);
    }

    L3Address broadcastAddr;
    broadcastAddr.set(Ipv4Address(0xFFFFFFFF));

    sendSimpleMessage(msg, broadcastAddr, hops);
}

void MamNodeApp::sendSimpleMessage(const char *msg, L3Address &dest, int hops)
{
    if (hops < 0) {
        throw cRuntimeError ("Invalid number of hops: %d", hops);
    }

    std::ostringstream str;
    str << msg;
    Packet *packet = new Packet(str.str().c_str());

    if (dontFragment)
        packet->addTagIfAbsent<FragmentationReq>()->setDontFragment(true);

    const auto& payload = makeShared<BMeshPacket>();
    payload->setChunkLength(B(1));
    payload->setHops(hops);
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    payload->setCreationTime(simTime());
    packet->insertAtBack(payload);

    emit(packetSentSignal, packet);
    socket.sendTo(packet, dest, destPort);
}

void MamNodeApp::sendFriendRequest()
{
    assert(lowPowerNode);
    assert(!friendNode);

    std::ostringstream str;
    str << FRIEND_REQUEST;
    Packet *packet = new Packet(str.str().c_str());

    if (dontFragment)
        packet->addTagIfAbsent<FragmentationReq>()->setDontFragment(true);

    const auto& payload = makeShared<BMeshPacket>();
    payload->setChunkLength(B(1));
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    packet->insertAtBack(payload);
    L3Address broadcastAddr;
    broadcastAddr.set(Ipv4Address(0xFFFFFFFF));

    emit(packetSentSignal, packet);
    socket.sendTo(packet, broadcastAddr, destPort);
}

void MamNodeApp::sendFriendEstablishedInternalMessage() {
    assert(lowPowerNode);
    assert(!friendNode);

    std::ostringstream str;
    str << "FRIEND_ESTABLISHED_INTERNAL";
    Packet *packet = new Packet(str.str().c_str());

    if (dontFragment)
        packet->addTagIfAbsent<FragmentationReq>()->setDontFragment(true);

    const auto& payload = makeShared<BMeshPacket>();
    payload->setChunkLength(B(1));
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    packet->insertAtBack(payload);
    L3Address broadcastAddr;
    broadcastAddr.set(Ipv4Address(0xFFFFFFFF));

    emit(packetSentSignal, packet);
    socket.sendTo(packet, broadcastAddr, destPort);
}

void MamNodeApp::sendFriendPoll() {
    assert(lowPowerNode);
    assert(!friendNode);
    L3Address empty;
    assert(connectedFriendNode != empty);

    std::ostringstream str;
    str << FRIEND_POLL;
    Packet *packet = new Packet(str.str().c_str());

    if (dontFragment)
        packet->addTagIfAbsent<FragmentationReq>()->setDontFragment(true);

    const auto& payload = makeShared<BMeshPacket>();
    payload->setChunkLength(B(1));
    payload->addTag<CreationTimeTag>()->setCreationTime(simTime());
    packet->insertAtBack(payload);

    emit(packetSentSignal, packet);
    socket.sendTo(packet, connectedFriendNode, destPort);
}

void MamNodeApp::sendData(Ptr<const BMeshPacket> bmeshData, L3Address &dest) {

    std::ostringstream str;
    str << DATA_SEND;
    Packet *packet = new Packet(str.str().c_str());

    if (dontFragment)
        packet->addTagIfAbsent<FragmentationReq>()->setDontFragment(true);

    packet->insertAtBack(bmeshData);

    emit(packetSentSignal, packet);
    emit(dataSentSignal, packet);
    numDataSent++;

    socket.sendTo(packet, dest, destPort);
}

void MamNodeApp::socketDataArrived(UdpSocket *socket, Packet *packet)
{
    // process incoming packet
    processPacket(packet);
}

void MamNodeApp::socketErrorArrived(UdpSocket *socket, Indication *indication)
{
    EV_WARN << "Ignoring UDP error report " << indication->getName() << endl;
    delete indication;
}

void MamNodeApp::socketClosed(UdpSocket *socket)
{
    if (operationalState == State::STOPPING_OPERATION)
        startActiveOperationExtraTimeOrFinish(par("stopOperationExtraTime"));

    EV_ERROR << "Socket Closed" << endl;

    L3Address empty;
    mobileSink = empty;
}

void MamNodeApp::refreshDisplay() const
{
    ApplicationBase::refreshDisplay();

    char buf[100];
    sprintf(buf, "rcvd: %d pks\nsent: %d pks", numReceived, numSent);
    getDisplayString().setTagArg("t", 0, buf);
}

void MamNodeApp::processPacket(Packet *pk)
{
    emit(packetReceivedSignal, pk);
    EV_INFO << "Received packet: " << UdpSocket::getReceivedPacketInfo(pk) << endl;
    delete pk;
    numReceived++;
}

void MamNodeApp::handleStartOperation(LifecycleOperation *operation)
{
    simtime_t start = std::max(startTime, simTime());
    if ((stopTime < SIMTIME_ZERO) || (start < stopTime) || (start == stopTime && startTime == stopTime)) {
        selfMsg->setKind(START);
        scheduleAt(start, selfMsg);
    }
}

void MamNodeApp::handleStopOperation(LifecycleOperation *operation)
{
    cancelEvent(selfMsg);
    socket.close();
    delayActiveOperationFinish(par("stopOperationTimeout"));
}

void MamNodeApp::handleCrashOperation(LifecycleOperation *operation)
{
    cancelEvent(selfMsg);
    socket.destroy();         //TODO  in real operating systems, program crash detected by OS and OS closes sockets of crashed programs.
}

unsigned int MamNodeApp::random_char()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    return dis(gen);
}

int MamNodeApp::disGen(bool second) {
    static std::random_device              rd;
    static std::mt19937                    gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis2(8, 11);

    if (!second)
        return dis(gen);

    return dis2(gen);
}

std::string MamNodeApp::generate_uuid_v4() {
    std::stringstream ss;
    int i;
    ss << std::hex;

    for (i = 0; i < 8; i++) {
        ss << disGen(false);
    }
    ss << "-";
    for (i = 0; i < 4; i++) {
        ss << disGen(false);
    }
    ss << "-4";
    for (i = 0; i < 3; i++) {
        ss << disGen(false);
    }
    ss << "-";
    ss << disGen(true);
    for (i = 0; i < 3; i++) {
        ss << disGen(false);
    }
    ss << "-";
    for (i = 0; i < 12; i++) {
        ss << disGen(false);
    };
    return ss.str();
}

std::string MamNodeApp::generate_hex(const unsigned int len)
{
    std::stringstream ss;
    for (unsigned int i = 0; i < len; i++) {
        const auto rc = random_char();
        std::stringstream hexstream;
        hexstream << std::hex << rc;
        auto hex = hexstream.str();
        ss << (hex.length() < 2 ? '0' + hex : hex);
    }
    return ss.str();
}

} // namespace inet
