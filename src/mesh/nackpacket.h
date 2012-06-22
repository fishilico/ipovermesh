#ifndef NACKPACKET_H
#define NACKPACKET_H

#include "../parser/gttpacket.h"
#include "../net/address.h"
#include "../net/sockpacket.h"

namespace iom{

    class NAckPacket : public SockPacket
    {
    public:
        NAckPacket(const Address& source, const Address& destination, const Address& nexthop);
        /**
         * @brief Parse a GTT packet
         * @param gttpkt
         */
        NAckPacket(const GTTPacket& gttpkt);

        /**
         * @brief Build the raw text query for this packet
         * @note Please read GTTPacket::build documentation
         */
        unsigned long build(unsigned char **newData) const;

        // Properties
        Address source;
        Address destination;
        Address nexthop;

    private:
        /**
         * @brief Private function to get a GTT packet
         * @param gttpkt (out) GTT packet
         */
        void fillGTTPacket(GTTPacket &gttpkt) const;
    };
}

#endif // NACKPACKET_H
