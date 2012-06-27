/**
 * @file netif.h
 * @brief Network interface management
 */

#ifndef IOM_NET_NETIF_H
#define IOM_NET_NETIF_H

#include <sys/types.h>
#include <ifaddrs.h>
#include <vector>
#include "address.h"

namespace iom
{
    class NetIf
    {
    public:
        NetIf();
        /**
         * @brief Build a NetIf from a result of getifaddrs
         * @param iface
         */
        NetIf(const struct ifaddrs& iface);

        NetIf(const NetIf& ni);
        NetIf& operator=(const NetIf& ni);

        friend std::ostream& operator<<(std::ostream& os, const NetIf& ni);

        const std::string& getName() const;

        const Address& getAddress() const;

        const Address& getBroadcast() const;

        /**
         * @brief Get all interfaces on the system
         */
        static std::vector<NetIf> getAllNet();

        /**
         * @brief Get all interfaces for a given name
         * @param name interface's name
         */
        static std::vector<NetIf> getByName(const std::string& name);

        /**
         * @brief Get all wifi interfaces which are up
         */
        static std::vector<NetIf> getWifiUp();

    private:
        // These properties are copies of struct ifaddrs, see man 3 getifaddrs
        std::string name;
        unsigned int flags;
        Address addr;
        Address netmask;
        Address broadcast;
    };
}

#endif /* IOM_NET_NETIF_H */

