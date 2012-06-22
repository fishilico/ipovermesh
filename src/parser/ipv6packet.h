#ifndef IPV6PACKET_H
#define IPV6PACKET_H

#include <boost/shared_array.hpp>
#include "../net/address.h"

namespace iom {

    class IPv6Packet
    {
    public:
        IPv6Packet(const boost::shared_array<unsigned char>& data, int size);
        const boost::shared_array<unsigned char>& getData() const;
        int getSize() const;
        const Address getSourceAddress() const;
        void setSourceAddress(const Address &address);
        const Address getDestinationAddress() const;
        void setDestinationAddress(const Address &address);

    private:
        std::string convertToHexa(unsigned char octet) const;
        unsigned char convertFromHexa(std::string hexa) const;
        boost::shared_array<unsigned char> data;
        int size;
    };
}

#endif // IPV6PACKET_H
