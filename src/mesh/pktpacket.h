/**
 * @file pktpacket.h
 * @brief PKT packet
 */

#ifndef IOM_NET_PKTPACKET_H
#define IOM_NET_PKTPACKET_H

#include "../parser/gttpacket.h"
#include "../parser/ipv6packet.h"
#include "../net/address.h"
#include "../net/sockpacket.h"

namespace iom
{
    class PktPacket : public SockPacket
    {
    public:
        PktPacket(const Address& source, const Address& destination,
                const Address& nexthop, unsigned int seq,
                unsigned long size, const boost::shared_array<unsigned char>& data);

        /**
         * @brief Parse a GTT packet
         * @param gttpkt
         */
        PktPacket(const GTTPacket& gttpkt);

        /**
         * @brief Encapsulate an IPv6 packet
         * @param ippkt IP packet
         * @param nexthop
         * @param seq Sequence number
         */
        PktPacket(const IPv6Packet& ippkt, const Address& nexthop, unsigned int seq);

        /**
         * @brief Build the raw text query for this packet
         * @note Please read GTTPacket::build documentation
         */
        unsigned long build(unsigned char **newData) const;

        // Properties
        Address source;
        Address destination;
        Address nexthop;
        unsigned int seq;

        unsigned long size;
        boost::shared_array<unsigned char> data;
    };
}

#endif /* IOM_NET_PKTPACKET_H */

