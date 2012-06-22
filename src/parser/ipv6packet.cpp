#include "ipv6packet.h"
#include "../core/exception.h"
#include "../core/log.h"

namespace iom {
    IPv6Packet::IPv6Packet(const boost::shared_array<unsigned char>& data, int size) :
        data(data),
        size(size)
    {
        if((data[0] & 0b01100000) != 0b01100000)
            throw FailException("IPv6Packet", "Data is not an IPv6 packet.");
    }

    const boost::shared_array<unsigned char>& IPv6Packet::getData() const {
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

    void IPv6Packet::setSourceAddress(const Address &address) {
        std::string addr = address.toString();
        for(int i = 0; i < 8; i++){
            data[8+2*i] = convertFromHexa(addr);
            addr = addr.substr(2);
            data[8+2*i+1] = convertFromHexa(addr);
            addr = addr.substr(2);
            if(i < 7)
                addr = addr.substr(1);
        }
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

    void IPv6Packet::setDestinationAddress(const Address &address) {
        std::string addr = address.toString();
        for(int i = 0; i < 8; i++){
            data[24+2*i] = convertFromHexa(addr);
            addr = addr.substr(2);
            data[24+2*i+1] = convertFromHexa(addr);
            addr = addr.substr(2);
            if(i < 7)
                addr = addr.substr(1);
        }
    }

    std::string IPv6Packet::convertToHexa(unsigned char octet) const {
        std::string result;
        unsigned char firstChar = octet >> 4;
        unsigned char secondChar = octet & 15;
        result += (firstChar < 10)?(firstChar + 0x30):(firstChar - 10 + 0x61);
        result += (secondChar < 10)?(secondChar + 0x30):(secondChar - 10 + 0x61);
        return result;
    }

    unsigned char IPv6Packet::convertFromHexa(std::string hexa) const {
        unsigned char result = 0;
        unsigned char firstChar = hexa[0];
        unsigned char secondChar = hexa[1];
        result += ((firstChar >= 0x61)?(firstChar - 0x61 + 10):(firstChar - 0x30)) << 4;
        result += (secondChar >= 0x61)?(secondChar - 0x61 + 10):(secondChar - 0x30);
        return result;
    }
}
