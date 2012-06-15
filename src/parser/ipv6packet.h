#ifndef IPV6PACKET_H
#define IPV6PACKET_H

#include "../net/address.h"

namespace iom {

    class IPv6Packet
    {
    public:
        IPv6Packet(unsigned char* data, int size);
        const unsigned char* getData() const;
        int getSize() const;
        const Address getSourceAddress() const;
        const Address getDestinationAddress() const;

    private:
        std::string convertToHexa(unsigned char octet) const;
        unsigned char* data;
        int size;
    };
}

#endif // IPV6PACKET_H
