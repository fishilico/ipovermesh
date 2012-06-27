#include "wifi.h"
#include "../ipovermesh.h"
#include "../parser/gttparser.h"

#define RREQUEST_EXPIRATION_DELAY boost::posix_time::seconds(60)
#define ACK_EXPIRATION_DELAY boost::posix_time::seconds(60)

namespace iom {
    Wifi::Wifi() :
        seq(0)
    {
        std::vector<NetIf> ifaces = NetIf::getWifiUp();
        for(std::vector<NetIf>::const_iterator i = ifaces.begin(); i != ifaces.end(); i++)
        {
            if(i->getAddress().getVersion() == 4)
            {
                interface = *i;
                SockAddress bindAddress(interface.getAddress(), 1337);
                server = new UDPServer(bindAddress);
                SockAddress broadcastAddress(interface.getBroadcast(),1337);
                broadcastSocket = new UDPSocket(broadcastAddress);
                return;
            }
        }
        throw FailException("Wifi", "No active wifi interface with IPv4 address");
        //TODO Calculer l'adresse du tun
    }

    Wifi::~Wifi()
    {
        delete server;
        delete broadcastSocket;
    }

    void Wifi::run()
    {
        GTTParser parser;
        int size;
        unsigned char data[2000];
        GTTPacket* packet;
        while(true)
        {
            try {
                if((size = server->recv(data, 2000)) == -1)
                {
                    log::error << "Wifi: Error while reading data." << log::errstd << log::endl;
                    throw MinorException("Wifi", "Error while reading data.");
                }
                parser.eat(data, size);
                while((packet = parser.getPacket()) != NULL)
                {
                    if(packet->method.compare("ACK"))
                        receiveAck(packet);
                    else if(packet->method.compare("NACK"))
                        receiveNAck(packet);
                    else if(packet->method.compare("RREQ"))
                        receiveRRequest(packet);
                    else if(packet->method.compare("RREP"))
                        receiveRReply(packet);
                    else if(packet->method.compare("PKT"))
                        receivePkt(packet);
                    else
                        log::error << "Wifi: Unknown packet method " << packet->method << log::endl;
                    delete packet;
                }
            }
            catch(MinorException &e) {}
            catch(ParserException &e) {}
        }
    }

    void Wifi::send(const IPv6Packet &packet)
    {
        Address destination = packet.getDestinationAddress();
        boost::shared_ptr<Route> route = routingTable.getRoute(destination);
        if(route.get() == 0)
        {
            std::map<Address, ptime>::iterator rreplyIt = pendingRReplies.find(destination);
            if(rreplyIt == pendingRReplies.end() || boost::posix_time::second_clock::local_time() - rreplyIt->second > RREQUEST_EXPIRATION_DELAY)
            {
                RRequestPacket rreq(address, destination, address, 1);
                rreq.send(*broadcastSocket);
                pendingRReplies.insert(std::pair<Address, ptime>(destination, boost::posix_time::second_clock::local_time()));
            }
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
        send(packet, route->nextHop);
    }

    void Wifi::clearOutdatedPackets()
    {
        while(true) {
            for(std::map<Address, ptime>::iterator it = pendingRReplies.begin(); it != pendingRReplies.end(); it++)
            {
                if(boost::posix_time::second_clock::local_time() - it->second > RREQUEST_EXPIRATION_DELAY)
                {
                    std::map<Address, std::queue<IPv6Packet> >::iterator packetQueue = packetsToSend.find(it->first);
                    packetsToSend.erase(packetQueue);
                    pendingRReplies.erase(it);
                }
            }
            std::map<ptime, sequenceIdentifier>::iterator sequencesIt = pendingAckSequences.begin();
            while(sequencesIt != pendingAckSequences.end() && boost::posix_time::second_clock::local_time() - sequencesIt->first > ACK_EXPIRATION_DELAY)
            {
                std::map<sequenceIdentifier, NAckPacket>::iterator nackIt = pendingAcks.find(sequencesIt->second);
                nackIt->second.send(*broadcastSocket);
                pendingAcks.erase(nackIt);
                pendingAckSequences.erase(sequencesIt);
            }
            sleep(10);
        }
    }

    void Wifi::send(const IPv6Packet &packet, const Address &nextHop)
    {
        PktPacket pkt(packet, nextHop, seq);
        seq++;
        NAckPacket nack(address, packet.getDestinationAddress(), address);
        send(pkt, nack);
    }

    void Wifi::send(const PktPacket &packet, const NAckPacket &nack)
    {
        packet.send(*broadcastSocket);
        sequenceIdentifier sequence(packet.source, packet.seq);
        pendingAcks.insert(std::pair<sequenceIdentifier,NAckPacket>(sequence, nack));
        pendingAckSequences.insert(std::pair<ptime, sequenceIdentifier >(boost::posix_time::second_clock::local_time(),sequence));
    }

    void Wifi::receiveAck(GTTPacket *packet)
    {
        AckPacket ack(*packet);
        if(ack.nexthop != address)
            return;
        std::map<std::pair<Address,int>, NAckPacket>::iterator nackIt = pendingAcks.find(sequenceIdentifier(ack.source, ack.seq));
        if(nackIt == pendingAcks.end())
        {
            log::error << "Wifi: Two ACKs received" << log::endl;
            return;
        }
        pendingAcks.erase(nackIt);
    }

    void Wifi::receiveNAck(GTTPacket *packet)
    {
        NAckPacket nack(*packet);
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
        nack.send(*broadcastSocket);
    }

    void Wifi::receivePkt(GTTPacket *packet)
    {
        PktPacket pkt(*packet);
        if(pkt.nexthop != address)
            return;
        if(pkt.destination == address)
            return;// TODO send to TUN
        boost::shared_ptr<Route> route = routingTable.getRoute(pkt.destination);
        NAckPacket nack(pkt.destination,pkt.source, route->nextHop);
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
        pkt.nexthop = route->nextHop;
        send(pkt, nack);
    }

    void Wifi::receiveRReply(GTTPacket *packet)
    {
        RReplyPacket rrep(*packet);
        if(rrep.nexthop != address)
            return;
        Route *toSource = new Route(rrep.source,rrep.sender,rrep.n);
        Route *toNextHop = new Route(rrep.sender,rrep.sender,rrep.n);
        routingTable.addRoute(boost::shared_ptr<Route>(toSource));
        routingTable.addRoute(boost::shared_ptr<Route>(toNextHop));
        if(rrep.destination == address)
        {
            std::map<Address, std::queue<IPv6Packet> >::iterator toSend = packetsToSend.find(rrep.source);
            if(toSend != packetsToSend.end())
            {
                while(toSend->second.empty())
                {
                    send(toSend->second.front(), rrep.sender);
                    toSend->second.pop();
                }
                packetsToSend.erase(toSend);
            }
            return;
        }
    }

    void Wifi::receiveRRequest(GTTPacket *packet)
    {
        RRequestPacket rreq(*packet);
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
            RReplyPacket rrep(address, rreq.source, address, rreq.sender, 1);
            rrep.send(*broadcastSocket);
            return;
        }
        boost::shared_ptr<Route> toDestination = routingTable.getRoute(rreq.destination);
        if(toDestination.get() != 0)
        {
            RReplyPacket rrep(rreq.destination, rreq.source, address, rreq.sender, toDestination->hops + 1);
            rrep.send(*broadcastSocket);
            return;
        }
        if(rreq.n < 100)
        {
            rreq.n++;
            rreq.send(*broadcastSocket);
        }
    }
}

