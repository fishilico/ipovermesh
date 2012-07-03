#include "wifi.h"
#include "ackpacket.h"
#include "rreplypacket.h"
#include "rrequestpacket.h"
#include "../core/common.h"
#include "../parser/gttparser.h"
#include "../net/ctlsocket.h"
#include <boost/interprocess/detail/atomic.hpp>

#define RREQUEST_EXPIRATION_DELAY boost::posix_time::seconds(60)
#define ACK_EXPIRATION_DELAY boost::posix_time::seconds(60)

namespace iom {
    unsigned int Wifi::IPprefixLen = 40;

    Wifi::Wifi(const NetIf &netiface)
    :seq(0)
    {
        interface = netiface;
        // Compute Tun address
        CtlSocket ifaceSock(interface.getName());
        unsigned char hwaddr[6];
        if (!ifaceSock.getHwAddr(hwaddr)) {
            throw FailException("Wifi", "Unable to retrieve an HW address");
        }
        address = Wifi::IPfromHw(hwaddr);

        // Enable broadcast
        ifaceSock.enableBroadcast(true);

        // Listen to this interface on UDP:1337
        SockAddress bindAddress(interface.getAddress(), 1337);
        server = new UDPServer(bindAddress);
        // Send messages to everyone on UDP:1337 (with broadcast)
        SockAddress broadcastAddress(interface.getBroadcast(), 1337);
        broadcastSocket = new UDPSocket(broadcastAddress, true);
        broadcastSocket->bindToDevice(interface.getName());

        srvThread = new boost::thread(boost::bind(&Wifi::recvRun, this));

        cleaningThread = new boost::thread(boost::bind(&Wifi::clearOutdatedPacketsLoop, this, 1));
    }

    Wifi::~Wifi() {
        close();
    }

    void Wifi::close() {
        recvQueue.close();
        if (server != NULL) {
            if (server->isBinded()) {
                // Quit UDP server in a good way
                BOOST_VERIFY(srvThread != NULL);
                server->close();
                srvThread->join();
                delete srvThread;
                srvThread = NULL;

                // And stop cleanup thread
                if (cleaningThread != NULL) {
                    cleaningThread->join();
                    delete cleaningThread;
                    cleaningThread = NULL;
                }
            }
            delete server;
            server = NULL;
        }
        if (broadcastSocket != NULL)
            delete broadcastSocket;
        broadcastSocket = NULL;
    }

    boost::shared_ptr<NetIf> Wifi::getWlanIfIp4Up() {
        std::vector<NetIf> ifaces = NetIf::getAllNet();
        for (std::vector<NetIf>::const_iterator i = ifaces.begin(); i != ifaces.end(); i++) {
            if (i->getName().find("wlan") != std::string::npos) {
                if (i->isUp() && i->getAddress().getVersion() == 4) {
                    return boost::shared_ptr<NetIf>(new NetIf(*i));
                }
            }
        }
        return boost::shared_ptr<NetIf>();
    }

    void Wifi::recvRun()
    {
        log::setThreadName("WifiRecv");
        try {
            GTTParser parser;
            int size;
            unsigned char data[2000];
            GTTPacket* packet;
            BOOST_ASSERT(server != NULL);

            // Run until someone closes the server
            while (server->isBinded())
            {
                try {
                    if((size = server->recv(data, 2000)) == -1)
                    {
                        log::error << "Wifi: Error while reading data. " << log::errstd << log::endl;
                        throw MinorException("Wifi", "Error while reading data");
                    }
                    if (!server->isBinded()) {
                        break;
                    }
                    if (MESH_DEBUG_WIFI)
                        log::debug << "[Wifi] Recv " << size<< " bytes" << log::endl;
                    parser.eat(data, size);
                    while((packet = parser.getPacket()) != NULL)
                    {
                        if(!packet->method.compare("ACK"))
                            receiveAck(packet);
                        else if(!packet->method.compare("NACK"))
                            receiveNAck(packet);
                        else if(!packet->method.compare("RREQ"))
                            receiveRRequest(packet);
                        else if(!packet->method.compare("RREP"))
                            receiveRReply(packet);
                        else if(!packet->method.compare("PKT"))
                            receivePkt(packet);
                        else
                            log::error << "Wifi: Unknown packet method " << packet->method << log::endl;
                        delete packet;
                    }
                }
                catch(MinorException &e) {}
                catch(ParserException &e) {}
            }
        } catch (Exception e) {
            log::error << "Wifi: error " << e << log::endl;
        }
    }

    void Wifi::send(const IPv6Packet &packet)
    {
        const Address& destination = packet.getDestinationAddress();
        boost::shared_ptr<Route> route = routingTable.getRoute(destination);
        if(route.get() == 0)
        {
            // No known route. Let's discover one !
            {
                boost::upgrade_lock<boost::shared_mutex> lock(rrepMut);
                std::map<Address, ptime>::iterator rreplyIt = pendingRReplies.find(destination);
                if(rreplyIt == pendingRReplies.end() || boost::posix_time::second_clock::local_time() - rreplyIt->second > RREQUEST_EXPIRATION_DELAY)
                {
                    // Send a route request
                    if (MESH_DEBUG_WIFI)
                        log::debug << "[Wifi] Route Request to " << destination << log::endl;
                    boost::upgrade_to_unique_lock<boost::shared_mutex> ulock(lock);
                    RRequestPacket rreq(address, destination, address, 1);
                    rreq.send(*broadcastSocket);
                    pendingRReplies.insert(std::pair<Address, ptime>(destination, boost::posix_time::second_clock::local_time()));
                }
            }

            // Add the packet in the waiting queue
            boost::unique_lock<boost::shared_mutex> ulock(pktMut);
            std::map<Address, std::queue<IPv6Packet> >::iterator it = packetsToSend.find(destination);
            std::queue<IPv6Packet> queue;
            if(it != packetsToSend.end())
                queue = it->second;
            queue.push(packet);
            if(it == packetsToSend.end())
                packetsToSend.insert(std::pair<Address, std::queue<IPv6Packet> >(destination,queue));
            else
                packetsToSend.insert(it, std::pair<Address, std::queue<IPv6Packet> >(destination,queue));
            return;
        }
        if (MESH_DEBUG_WIFI)
            log::debug << "[Wifi] Send to " << route->nextHop << " for " << destination << log::endl;
        send(packet, route->nextHop);
    }

    void Wifi::send(const IPv6Packet &packet, const Address &nextHop)
    {
        boost::uint32_t pktSeq = boost::interprocess::ipcdetail::atomic_inc32(&seq);
        PktPacket pkt(packet, nextHop, pktSeq);
        NAckPacket nack(address, packet.getDestinationAddress(), address);
        send(pkt, nack);
    }

    void Wifi::send(const PktPacket &packet, const NAckPacket &nack)
    {
        packet.send(*broadcastSocket);
        sequenceIdentifier sequence(packet.source, packet.seq);
        boost::unique_lock<boost::shared_mutex> lock(ackMut);
        pendingAcks.insert(std::pair<sequenceIdentifier,NAckPacket>(sequence, nack));
        pendingAckSequences.insert(std::pair<ptime, sequenceIdentifier >(boost::posix_time::second_clock::local_time(),sequence));
    }

    boost::shared_ptr<IPv6Packet> Wifi::recv() {
        return recvQueue.pop();
    }

    const Address& Wifi::getAddress() const {
        return address;
    }

    const NetIf& Wifi::getInterface() const {
        return interface;
    }

    void Wifi::clearOutdatedPackets()
    {
        {
            boost::upgrade_lock<boost::shared_mutex> lock(rrepMut);
            for(std::map<Address, ptime>::iterator it = pendingRReplies.begin(); it != pendingRReplies.end(); it++)
            {
                if(boost::posix_time::second_clock::local_time() - it->second > RREQUEST_EXPIRATION_DELAY)
                {
                    boost::unique_lock<boost::shared_mutex> ulockPkt(pktMut);
                    std::map<Address, std::queue<IPv6Packet> >::iterator packetQueue = packetsToSend.find(it->first);
                    packetsToSend.erase(packetQueue);
                    boost::upgrade_to_unique_lock<boost::shared_mutex> ulock(lock);
                    pendingRReplies.erase(it);
                }
            }
        }
        {
            boost::unique_lock<boost::shared_mutex> lock(ackMut);
            std::map<ptime, sequenceIdentifier>::iterator sequencesIt = pendingAckSequences.begin();
            while(sequencesIt != pendingAckSequences.end() && boost::posix_time::second_clock::local_time() - sequencesIt->first > ACK_EXPIRATION_DELAY)
            {
                std::map<sequenceIdentifier, NAckPacket>::iterator nackIt = pendingAcks.find(sequencesIt->second);
                nackIt->second.send(*broadcastSocket);
                pendingAcks.erase(nackIt);
                pendingAckSequences.erase(sequencesIt);
            }
        }
    }

    void Wifi::clearOutdatedPacketsLoop(unsigned int seconds)
    {
        while (server->isBinded()) {
            clearOutdatedPackets();
            sleep(seconds);
        }
    }

    void Wifi::receiveAck(GTTPacket *packet)
    {
        AckPacket ack(*packet);
        if (MESH_DEBUG_WIFI)
            log::debug << "[Wifi] Recv ACK from " << ack.destination << log::endl;
        boost::unique_lock<boost::shared_mutex> lock(ackMut);
        std::map<std::pair<Address,int>, NAckPacket>::iterator nackIt = pendingAcks.find(sequenceIdentifier(ack.source, ack.seq));
        if(nackIt == pendingAcks.end())
            return;
        pendingAcks.erase(nackIt);
    }

    void Wifi::receiveNAck(GTTPacket *packet)
    {
        NAckPacket nack(*packet);
        if (MESH_DEBUG_WIFI)
            log::debug << "[Wifi] Recv NACK for " << nack.nexthop << log::endl;
        if(nack.nexthop != address)
            return;
        routingTable.invalidateRoute(nack.source);
        boost::shared_ptr<Route> route = routingTable.getRoute(nack.destination);
        if(nack.destination == address)
            return;
        if(route.get() == 0)
        {
            log::error << "Wifi: No route to forward NACK" << log::endl;
            return;
        }
        nack.nexthop = route->nextHop;
        if (MESH_DEBUG_WIFI)
            log::debug << "[Wifi] Forward NACK for " << nack.nexthop << " to " << nack.nexthop << log::endl;
        nack.send(*broadcastSocket);
    }

    void Wifi::receivePkt(GTTPacket *packet)
    {
        PktPacket pkt(*packet);
        if(pkt.nexthop != address) {
            // Ignore Mesh unicast from Wifi broadcast
            return;
        }
        if (MESH_DEBUG_WIFI)
            log::debug << "[Wifi] Recv PKT for " << pkt.nexthop << ", send ACK" << log::endl;
        AckPacket ack(pkt.source, pkt.destination, pkt.nexthop, pkt.seq);
        ack.send(*broadcastSocket);
        if(pkt.destination == address) {
            log::debug << "[Wifi] Recv IPv6 packet " << pkt.size << " bytes from " << pkt.source << log::endl;
            IPv6Packet *pkt6 = new IPv6Packet(pkt.data, pkt.size);
            //recvQueue->push(boost::shared_ptr<const IPv6Packet>(pkt6));
            recvQueue.push(pkt6);
            return;
        }
        boost::shared_ptr<Route> route = routingTable.getRoute(pkt.destination);
        NAckPacket nack(pkt.destination, pkt.source, route->nextHop);
        if(route.get() == 0)
        {
            log::error << "Wifi: No route to forward PKT" << log::endl;
            route = routingTable.getRoute(pkt.source);
            if(route.get() == 0)
            {
                log::error << "Wifi: No route to PKT source to send NACK" << log::endl;
                return;
            }
            nack.send(*broadcastSocket);
            return;
        }
        if (MESH_DEBUG_WIFI)
            log::debug << "[Wifi] Forward pkt to " << route->nextHop << log::endl;
        pkt.nexthop = route->nextHop;
        send(pkt, nack);
    }

    void Wifi::receiveRReply(GTTPacket *packet)
    {
        RReplyPacket rrep(*packet);
        if(rrep.nexthop != address)
            return;
        if (MESH_DEBUG_WIFI)
            log::debug << "[Wifi] Recv Route reply from " << rrep.source << log::endl;
        Route *toSource = new Route(rrep.source,rrep.sender,rrep.n);
        Route *toNextHop = new Route(rrep.sender,rrep.sender,rrep.n);
        routingTable.addRoute(boost::shared_ptr<Route>(toSource));
        routingTable.addRoute(boost::shared_ptr<Route>(toNextHop));
        if(rrep.destination == address)
        {
            boost::upgrade_lock<boost::shared_mutex> lock(pktMut);
            std::map<Address, std::queue<IPv6Packet> >::iterator toSend = packetsToSend.find(rrep.source);
            if(toSend != packetsToSend.end())
            {
                while(toSend->second.empty())
                {
                    send(toSend->second.front(), rrep.sender);
                    boost::upgrade_to_unique_lock<boost::shared_mutex> ulock(lock);
                    toSend->second.pop();
                }
                boost::upgrade_to_unique_lock<boost::shared_mutex> ulock(lock);
                packetsToSend.erase(toSend);
            }
            return;
        }
    }

    void Wifi::receiveRRequest(GTTPacket *packet)
    {
        RRequestPacket rreq(*packet);
        if (MESH_DEBUG_WIFI)
            log::debug << "[Wifi] Recv Route request from " << rreq.sender
                << " to " << rreq.destination << log::endl;

        boost::shared_ptr<Route> toSender(new Route(rreq.sender, rreq.sender, 1));
        routingTable.addRoute(toSender);
        boost::shared_ptr<Route> toSource = routingTable.getRoute(rreq.source);
        if(toSource.get() != 0)
        {
            if(toSource->hops < rreq.n)
                return;
        }
        toSource = boost::shared_ptr<Route>(new Route(rreq.source, rreq.sender, rreq.n));
        routingTable.addRoute(toSource);
        if(rreq.destination == address)
        {
            if (MESH_DEBUG_WIFI)
                log::debug << "[Wifi] Send Route reply" << log::endl;
            RReplyPacket rrep(address, rreq.source, address, rreq.sender, 1);
            rrep.send(*broadcastSocket);
            return;
        }
        boost::shared_ptr<Route> toDestination = routingTable.getRoute(rreq.destination);
        if(toDestination.get() != 0)
        {
            if (MESH_DEBUG_WIFI)
                log::debug << "[Wifi] Send Route reply" << log::endl;
            RReplyPacket rrep(rreq.destination, rreq.source, address, rreq.sender, toDestination->hops + 1);
            rrep.send(*broadcastSocket);
            return;
        }
        if(rreq.n < 100)
        {
            if (MESH_DEBUG_WIFI)
                log::debug << "[Wifi] Forward Route request" << log::endl;
            rreq.n++;
            rreq.send(*broadcastSocket);
        }
    }

    Address Wifi::IPfromHw(const unsigned char hwaddr[6]) {
        struct sockaddr_in6 saddr;
        unsigned char *ipv6 = saddr.sin6_addr.s6_addr;
        saddr.sin6_family = AF_INET6;
        saddr.sin6_flowinfo = 0;
        saddr.sin6_scope_id = 0;
        saddr.sin6_port = 0;

        // Prefix fd00:0da1:0000::/40
        ipv6[0] = 0xfd;
        ipv6[1] = 0;
        ipv6[2] = 0x0d;
        ipv6[3] = 0xa1;
        ipv6[4] = 0;
        ipv6[5] = 0;
        // Random bytes
        ipv6[6] = rand() & 0xff;
        ipv6[7] = rand() & 0xff;
        // MAC address
        ipv6[8] = hwaddr[0];
        ipv6[9] = hwaddr[1];
        ipv6[10] = hwaddr[2];
        ipv6[11] = 0xff;
        ipv6[12] = 0xfe;
        ipv6[13] = hwaddr[3];
        ipv6[14] = hwaddr[4];
        ipv6[15] = hwaddr[5];
        return Address((struct sockaddr*) &saddr);
    }
}
