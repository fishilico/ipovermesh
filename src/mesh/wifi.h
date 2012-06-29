#ifndef IOM_MESH_WIFI_H
#define IOM_MESH_WIFI_H

#include "nackpacket.h"
#include "pktpacket.h"
#include "routingtable.h"
#include "../core/blocking-queue.h"
#include "../net/udpserver.h"
#include "../net/netif.h"
#include "../parser/gttpacket.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <map>

namespace iom {

    class Wifi : public boost::noncopyable
    {
    public:

        /**
         * @brief Constructor
         * @param recvQueue Queue where packets from the WiFi are put
         */
        Wifi();
        ~Wifi();

        void close();

        /**
         * @brief Attempts to send an IPv6Packet on the mesh
         * @details If a route exists to the destination, packet is sent.
         * Otherwise, an attempt is made to discover the route before sending it.
         * If route is invalid, packet is lost.
         *
         * @param packet IPv6 payload
         */
        void send(const IPv6Packet &packet);

        /**
         * @brief Wait for a packet to be received
         * @return an IPv6 packet, or NULL if there was something wrong
         */
        boost::shared_ptr<IPv6Packet> recv();

        /**
         * @brief Return IPv6 address computed from the MAC address
         * @return
         */
        const Address& getAddress() const;

        /**
         * @brief Get interface
         */
        const NetIf& getInterface() const;

    private:
        typedef boost::posix_time::ptime ptime;
        typedef std::pair<Address, int> sequenceIdentifier;

        /**
         * @brief Other thread to receive data from the WiFi interface
         */
        void recvRun();
        boost::thread* srvThread;

        /**
         * @brief Clears outdated packets
         */
        void clearOutdatedPackets();
        /**
         * @brief Clears outdated packets at regular intervals
         * @param seconds time to sleep
         */
        void clearOutdatedPacketsLoop(unsigned int seconds);
        boost::thread* cleaningThread;

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
        boost::uint32_t seq;

        // Queue where received messages are saved
        BlockingQueue<IPv6Packet> recvQueue;

        boost::shared_mutex rrepMut;
        std::map<Address, ptime> pendingRReplies;
        boost::shared_mutex ackMut;
        std::map<ptime, sequenceIdentifier > pendingAckSequences;
        std::map<sequenceIdentifier, NAckPacket> pendingAcks;
        boost::shared_mutex pktMut;
        std::map<Address, std::queue<IPv6Packet> > packetsToSend;
    };
}

#endif // IOM_MESH_WIFI_H
