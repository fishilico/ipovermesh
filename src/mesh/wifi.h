#ifndef WIFI_H
#define WIFI_H

#include "../net/udpserver.h"
#include "../net/netif.h"
#include "../parser/gttpacket.h"
#include "routingtable.h"
#include "../ipovermesh.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <map>

namespace iom {

    class Wifi : public boost::noncopyable
    {
    public:
        Wifi();
        ~Wifi();
        void run();
        /**
         * @brief Attempts to send an IPv6Packet on the mesh
         * @details If a route exists to the destination, packet is sent. Otherwise, an attempt is made to discover the route before sending it. If route is invalid, packet is lost.
         * @param packet
         */
        void send(const IPv6Packet &packet);
        /**
         * @brief Clears outdated packets at regular intervals
         */
        void clearOutdatedPackets();

    private:
        typedef boost::posix_time::ptime ptime;
        typedef std::pair<Address, int> sequenceIdentifier;
        /**
         * @brief Sends an IPv6Packet on the mesh
         * @param packet
         * @param nextHop
         */
        void send(const IPv6Packet &packet, const Address &nextHop);
        /**
         * @brief Sends a PktPacket on the mesh, recording nack if no ACK is received from next hop.
         * @param packet
         * @param nack
         */
        void send(const PktPacket &packet, const NAckPacket &nack);
        void receiveAck(GTTPacket *packet);
        void receiveNAck(GTTPacket *packet);
        void receiveRRequest(GTTPacket *packet);
        void receiveRReply(GTTPacket *packet);
        void receivePkt(GTTPacket *packet);

        Address address;
        UDPServer* server;
        UDPSocket* broadcastSocket;
        NetIf interface;
        RoutingTable routingTable;
        unsigned int seq;

        std::map<Address, ptime> pendingRReplies;
        std::map<ptime, sequenceIdentifier > pendingAckSequences;
        std::map<sequenceIdentifier, NAckPacket> pendingAcks;
        std::map<Address, std::queue<IPv6Packet> > packetsToSend;
    };
}

#endif // WIFI_H
