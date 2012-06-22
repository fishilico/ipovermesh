/**
 * @file rreplypacket.h
 * @brief Route Reply packet
 */

#ifndef IOM_NET_RREPLYPACKET_H
#define IOM_NET_RREPLYPACKET_H

#include "../parser/gttpacket.h"
#include "../net/address.h"
#include "../net/sockpacket.h"

namespace iom
{
    class RReplyPacket : public SockPacket
    {
    public:
        RReplyPacket(const Address& source, const Address& destination,
                const Address& sender, const Address& nexthop, unsigned int n);

        /**
         * @brief Parse a GTT packet
         * @param gttpkt
         */
        RReplyPacket(const GTTPacket& gttpkt);

        /**
         * @brief Build the raw text query for this packet
         * @note Please read GTTPacket::build documentation
         */
        unsigned long build(unsigned char **newData) const;

        // Properties
        Address source;
        Address destination;
        Address sender;
        Address nexthop;
        unsigned int n;
    private:
        /**
         * @brief Private function to get a GTT packet
         * @param gttpkt (out) GTT packet
         */
        void fillGTTPacket(GTTPacket &gttpkt) const;
    };
}

#endif /* IOM_NET_RREPLYPACKET_H */

