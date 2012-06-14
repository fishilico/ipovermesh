#include "netif.h"
#include "../core/common.h"
#include <sstream>
#include <sys/ioctl.h>
#include <net/if.h>


namespace iom
{

    NetIf::NetIf()
    :flags(0) {
        // Everything is correctly initialized by default but flags...
    }

    NetIf::NetIf(const struct ifaddrs& iface)
    :name(iface.ifa_name), flags(iface.ifa_flags) {
        if (iface.ifa_addr->sa_family == AF_INET || iface.ifa_addr->sa_family == AF_INET6) {
            if (iface.ifa_addr != NULL)
                addr = Address(iface.ifa_addr);
            if (iface.ifa_netmask != NULL)
                netmask = Address(iface.ifa_netmask);
        }
        if (flags & IFF_BROADCAST && iface.ifa_ifu.ifu_broadaddr != NULL) {
            broadcast = Address(iface.ifa_ifu.ifu_broadaddr);
        }
    }

    NetIf::NetIf(const NetIf& ni)
    :name(ni.name), flags(ni.flags), addr(ni.addr),
    netmask(ni.netmask), broadcast(ni.broadcast) {
    }

    NetIf& NetIf::operator=(const NetIf& ni) {
        if (this != &ni) {
            this->name = ni.name;
            this->flags = ni.flags;
            this->addr = ni.addr;
            this->netmask = ni.netmask;
            this->broadcast = ni.broadcast;
        }
        return *this;
    }

    std::ostream& operator<<(std::ostream& os, const NetIf& ni) {
        os << ni.name << " (" << ni.addr << "/" << ni.netmask;
        if (!ni.broadcast.empty())
            os << " bdcast " << ni.broadcast;
        os << ")";
        return os;
    }

    const std::string& NetIf::getName() const {
        return name;
    }

    std::vector<NetIf> NetIf::getAllNet() {
        std::vector<NetIf> ifaces;
        struct ifaddrs *ifAddrStruct = NULL;
        struct ifaddrs *ifa = NULL;

        getifaddrs(&ifAddrStruct);
        for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr != NULL
                && (ifa->ifa_addr->sa_family == AF_INET || ifa->ifa_addr->sa_family == AF_INET6)) {
                ifaces.push_back(NetIf(*ifa));
            }
        }
        if (ifAddrStruct != NULL)
            freeifaddrs(ifAddrStruct);
        return ifaces;
    }

    std::vector<NetIf> NetIf::getByName(const std::string& name) {
        std::vector<NetIf> ifaces;
        struct ifaddrs *ifAddrStruct = NULL;
        struct ifaddrs *ifa = NULL;

        getifaddrs(&ifAddrStruct);
        for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
            if (name.compare(ifa->ifa_name) == 0) {
                ifaces.push_back(NetIf(*ifa));
            }
        }
        if (ifAddrStruct != NULL)
            freeifaddrs(ifAddrStruct);
        return ifaces;
    }

    std::vector<NetIf> NetIf::getWifiUp() {

        std::vector<NetIf> ifaces;
        struct ifaddrs *ifAddrStruct = NULL;
        struct ifaddrs *ifa = NULL;

        getifaddrs(&ifAddrStruct);
        for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr != NULL && (ifa->ifa_flags & IFF_UP)
                && (ifa->ifa_addr->sa_family == AF_INET || ifa->ifa_addr->sa_family == AF_INET6)) {
                if (std::string(ifa->ifa_name).find("wlan") != std::string::npos)
                    ifaces.push_back(NetIf(*ifa));
            }
        }
        if (ifAddrStruct != NULL)
            freeifaddrs(ifAddrStruct);
        return ifaces;
    }
}
