#include "address.h"
#include "../core/common.h"
#include <sstream>
#include <string.h>
#include <ifaddrs.h>

namespace iom
{

    Address::Address()
    :ip(), ipVersion(0) {
    }

    Address::Address(const std::string& ip, int ipVersion)
    :ip(ip), ipVersion(ipVersion) {
        std::string portstr = "";
        if (ip.empty())
            return;
        size_t iplen = ip.length();

        // Remove brackets around IPv6 addresses
        if (ip.at(0) == '[') {
            this->ipVersion = 6;
            if (ip.at(iplen - 1) != ']') {
                // TODO: Better sanity checks
                log::fatal << "Unknown IPv6 " << ip << log::endl;
                throw ParserException("Address", "Invalid IPv6 address");
            }
            this->ip = ip.substr(1, iplen - 2);
        }

        // Guess IP version
        if (this->ipVersion == 0) {
            if (ip.find(':') != std::string::npos)
                this->ipVersion = 6;
            else
                this->ipVersion = 4;
        }
    }

    Address::Address(const struct sockaddr *saddr) {
        BOOST_ASSERT(saddr != NULL);
        char ipBuffer[INET6_ADDRSTRLEN + 1];

        if (saddr->sa_family == AF_INET) {
            // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *) saddr;
            inet_ntop(AF_INET, &(ipv4->sin_addr), ipBuffer, sizeof (ipBuffer));
            this->ip.assign(ipBuffer);
            this->ipVersion = 4;
        } else if (saddr->sa_family == AF_INET6) {
            // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *) saddr;
            inet_ntop(AF_INET6, &(ipv6->sin6_addr), ipBuffer, sizeof (ipBuffer));
            this->ip.assign(ipBuffer);
            this->ipVersion = 6;
        } else {
            log::fatal << "You have just invented a new IP version " \
                << "without giving me information about how to handle it\n" \
                << "The version is: " << saddr->sa_family << "\n" \
                << "IPv4 is: " << AF_INET << "\n" \
                << "IPv6 is: " << AF_INET6 \
                << log::endl;
            throw FailException("Address", "Bad IP Version");
        }
    }

    Address::Address(const Address& addr)
    :ip(addr.ip), ipVersion(addr.ipVersion) {
    }

    Address& Address::operator=(const Address& addr) {
        if (this != &addr) {
            this->ip = addr.ip;
            this->ipVersion = addr.ipVersion;
        }
        return *this;
    }

    int Address::getVersion() const {
        return ipVersion;
    }

    bool Address::empty() const {
        return ip.empty();
    }

    void Address::getSockAddr(struct sockaddr *saddr, unsigned short port) const {
        BOOST_ASSERT(saddr != NULL);
        BOOST_ASSERT(this->ipVersion > 0);
        if (this->ipVersion == 4) {
            // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *) saddr;
            memset(&(ipv4->sin_zero), 0, sizeof (ipv4->sin_zero));
            ipv4->sin_family = AF_INET;
            inet_pton(AF_INET, ip.c_str(), &(ipv4->sin_addr));
            ipv4->sin_port = htons(port);
        } else if (this->ipVersion == 6) {
            // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *) saddr;
            ipv6->sin6_family = AF_INET6;
            ipv6->sin6_flowinfo = 0;
            ipv6->sin6_scope_id = 0;
            inet_pton(AF_INET6, ip.c_str(), &(ipv6->sin6_addr));
            ipv6->sin6_port = htons(port);
        } else {
            throw FailException("Address", "Bad IP Version");
        }
    }

    std::ostream& operator<<(std::ostream& os, const Address& addr) {
        if (addr.ip.length())
            os << addr.ip;
        else
            os << '*';
        return os;
    }

    std::string Address::toString(bool addBrackets) const {
        if (addBrackets && ipVersion == 6) {
            std::ostringstream stream;
            stream << '[' << ip << ']';
            return stream.str();
        } else {
            return ip;
        }
    }

    int Address::compare(const Address& addr) const {
        if (ipVersion < addr.ipVersion) return -1;
        if (ipVersion > addr.ipVersion) return 1;
        // TODO: compare with bitmasks
        int c = ip.compare(addr.ip);
        if (c != 0) return c;
        return 0;
    }

    bool operator==(const Address& addr1, const Address& addr2) {
        return addr1.compare(addr2) == 0;
    }

    bool operator!=(const Address& addr1, const Address& addr2) {
        return addr1.compare(addr2) != 0;
    }

    bool operator<(const Address& addr1, const Address& addr2) {
        return addr1.compare(addr2) < 0;
    }

    Address Address::fromHw(const unsigned char hwaddr[6]) {
        struct sockaddr_in6 saddr;
        unsigned char *ipv6 = saddr.sin6_addr.s6_addr;
        saddr.sin6_family = AF_INET6;
        saddr.sin6_flowinfo = 0;
        saddr.sin6_scope_id = 0;
        saddr.sin6_port = 0;

        // Prefix fe80::/16
        ipv6[0] = 0xfe;
        ipv6[1] = 0x80;
        // Random bytes
        ipv6[2] = rand() & 0xff;
        ipv6[3] = rand() & 0xff;
        ipv6[4] = rand() & 0xff;
        ipv6[5] = rand() & 0xff;
        ipv6[6] = rand() & 0xff;
        ipv6[7] = rand() & 0xff;
        // MAC address
        ipv6[8] = hwaddr[0];
        ipv6[9] = hwaddr[1];
        ipv6[10] = hwaddr[2];
        ipv6[11] = 0xff;
        ipv6[12] = 0xfe;
        ipv6[13] = hwaddr[3];
        ipv6[14] = hwaddr[4];
        ipv6[15] = hwaddr[5];
        return Address((struct sockaddr*) &saddr);
    }
}
