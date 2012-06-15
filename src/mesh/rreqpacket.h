/**
 * @file rreqpacket.h
 * @brief Route Request packet
 */

#ifndef RREQPACKET_H
#define RREQPACKET_H

#include "../parser/gttpacket.h"
#include "../net/address.h"
#include "../net/sockpacket.h"

namespace iom
{
    class RReqPacket : public SockPacket
    {
    public:
        RReqPacket(const Address& source, const Address& destination, unsigned int n);

        /**
         * @brief Parse a GTT packet
         * @param gttpkt
         */
        RReqPacket(const GTTPacket& gttpkt);

        RReqPacket(const RReqPacket& pkt);

        const RReqPacket& operator=(const RReqPacket& pkt);

        /**
         * @brief Build the raw text query for this packet
         * @note Please read GTTPacket::build documentation
         */
        unsigned long build(char **newData) const;

        // Properties
        Address source;
        Address destination;
        unsigned int n;
    private:
        /**
         * @brief Private function to get a GTT packet
         * @param gttpkt (out) GTT packet
         */
        void fillGTTPacket(GTTPacket &gttpkt) const;
    };
}

#endif /* RREQPACKET_H */

