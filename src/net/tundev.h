/**
 * @file tundev.h
 * @brief Tun network device management system to receive or emit local IP packets
 */

#ifndef TUNDEV_H
#define TUNDEV_H

#include "../core/common.h"
#include "address.h"
#include "../parser/ipv6packet.h"

namespace iom
{
    class Tundev
    {
    public:
        /**
         * @brief Open this device
         * @param devname Device name
         */
        Tundev(const std::string& devname = "");
        ~Tundev();
        /**
         * @brief Configures the MTU for this device
         * @param mtu
         */
        void setMTU(int mtu);
        /**
         * @brief Sets the IPv6 for this device
         * @param ip
         */
        void setIPv6(const Address &ip);
        /**
         * @brief Activates this device
         */
        void activate();
        /**
         * @brief Reads at most __nbytes from the device in __buf and returns the number of bytes read.
         * @param __buf
         * @param __nbytes
         * @return
         */
        int read(void *__buf, size_t __nbytes);
        /**
         * @brief Reads an IPv6 packet from the device and returns it.
         * @return
         */
        IPv6Packet readPacket();
        /**
         * @brief Writes at most __nbytes to the device from __buf and returns the number of bytes written.
         * @param __buf
         * @param __nbytes
         * @return
         */
        int write(const void *__buf, size_t __nbytes);
        /**
         * @brief Writes an IPv6 packet to the device
         * @param data
         * @return True on success, false otherwise.
         */
        bool writePacket(const IPv6Packet &data);


    private:
        // File descriptor to device
        int fd;
        // Device name
        std::string devname;
        int mtu;
    };
}

#endif /* TUNDEV_H */

