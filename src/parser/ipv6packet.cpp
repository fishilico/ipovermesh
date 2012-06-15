#include "ipv6packet.h"
#include "../core/exception.h"
#include "../core/log.h"

namespace iom {
    IPv6Packet::IPv6Packet(unsigned char* data, int size) :
        data(data),
        size(size)
    {
        if((data[0] & 0b01100000) != 0b01100000)
            throw FailException("IPv6Packet", "Data is not an IPv6 packet.");
    }

    const unsigned char* IPv6Packet::getData() const {
        return data;
    }

    int IPv6Packet::getSize() const {
        return size;
    }

    const Address IPv6Packet::getSourceAddress() const {
        std::string address;
        for(int i = 0; i < 8; i++)
        {
            address += convertToHexa(data[8+2*i]);
            address += convertToHexa(data[8+2*i+1]);
            if(i < 7)
                address += ':';
        }
        return Address(address);
    }

    const Address IPv6Packet::getDestinationAddress() const {
        std::string address;
        for(int i = 0; i < 8; i++)
        {
            address += convertToHexa(data[24+2*i]);
            address += convertToHexa(data[24+2*i+1]);
            if(i < 7)
                address += ':';
        }
        return Address(address);
    }

    std::string IPv6Packet::convertToHexa(unsigned char octet) const {
        std::string result;
        unsigned char firstChar = octet >> 4;
        unsigned char secondChar = octet & 15;
        result += (firstChar < 10)?(firstChar + 0x30):(firstChar - 10 + 0x61);
        result += (secondChar < 10)?(secondChar + 0x30):(secondChar - 10 + 0x61);
        return result;
    }
}
