/**
 * @file tundev.h
 * @brief Tun network device management system to receive or emit local IP packets
 */

#ifndef TUNDEV_H
#define TUNDEV_H

#include "../core/common.h"

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

    private:
        // File descriptor to device
        int fd;
        // Device name
        std::string devname;
    };
}

#endif /* TUNDEV_H */

