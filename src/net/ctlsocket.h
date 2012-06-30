/**
 * @file ctlsocket.h
 * @brief Control socket
 */

#ifndef IOM_NET_CTLSOCKET_H
#define IOM_NET_CTLSOCKET_H

#include <boost/noncopyable.hpp>
#include <net/if.h>
#include <string>
#include <unistd.h>
#include "address.h"

namespace iom
{
    /**
     * @class CtlSocket
     * @brief Socket for IOCTL on some devices
     */
    class CtlSocket : private boost::noncopyable
    {
    public:
        CtlSocket(const std::string& devname, int ipVersion = 4);
        ~CtlSocket();

        /**
         * @brief Set Maximal Transmission Unit of the device interface
         * @param mtu
         * @return true on success
         */
        bool setMTU(int mtu);

        /**
         * @brief Get Maximal Transmission Unit
         * @return MTU, or 0 if ioctl failed
         */
        int getMTU();

        /**
         * @brief Get hardware address
         * @param addr hardware address (6 bytes)
         * @return false on error
         */
        bool getHwAddr(unsigned char hwaddr[6]);

        bool activateInterface(bool up);

        bool enableBroadcast(bool enable);

    private:
        bool ioctlReq(int request, struct ifreq& ifr);

        std::string devname;
        int sock;
    };
}

#endif /* IOM_NET_CTLSOCKET_H */

