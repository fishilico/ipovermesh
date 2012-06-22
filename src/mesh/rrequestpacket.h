/**
 * @file rrequestpacket.h
 * @brief Route Request packet
 */

#ifndef IOM_NET_RREQUESTPACKET_H
#define IOM_NET_RREQUESTPACKET_H

#include "../parser/gttpacket.h"
#include "../net/address.h"
#include "../net/sockpacket.h"

namespace iom
{
    class RRequestPacket : public SockPacket
    {
    public:
        RRequestPacket(const Address& source, const Address& destination,
                const Address& sender, unsigned int n);

        /**
         * @brief Parse a GTT packet
         * @param gttpkt
         */
        RRequestPacket(const GTTPacket& gttpkt);

        RRequestPacket(const RRequestPacket& pkt);

        const RRequestPacket& operator=(const RRequestPacket& pkt);

        /**
         * @brief Build the raw text query for this packet
         * @note Please read GTTPacket::build documentation
         */
        unsigned long build(char **newData) const;

        // Properties
        Address source;
        Address destination;
        Address sender;
        unsigned int n;
    private:
        /**
         * @brief Private function to get a GTT packet
         * @param gttpkt (out) GTT packet
         */
        void fillGTTPacket(GTTPacket &gttpkt) const;
    };
}

#endif /* IOM_NET_RREQPACKET_H */

