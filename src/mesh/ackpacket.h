#ifndef ACKPACKET_H
#define ACKPACKET_H

#include "../parser/gttpacket.h"
#include "../net/address.h"
#include "../net/sockpacket.h"

namespace iom{

    class AckPacket : public SockPacket
    {
    public:
        AckPacket(const Address& source, const Address& destination, const Address& nexthop, unsigned int seq);
        /**
         * @brief Parse a GTT packet
         * @param gttpkt
         */
        AckPacket(const GTTPacket& gttpkt);

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

    private:
        /**
         * @brief Private function to get a GTT packet
         * @param gttpkt (out) GTT packet
         */
        void fillGTTPacket(GTTPacket &gttpkt) const;
    };
}

#endif // ACKPACKET_H
