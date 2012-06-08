#include "tundev.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <string>


namespace iom
{

    Tundev::Tundev(const std::string& devname)
    {
        fd = ::open("/dev/net/tun", O_RDWR);
        if (fd < 0)
            throw ErrException("TunDev", "Unable to open /dev/net/tun");

        struct ifreq ifr;
        memset(&ifr, 0, sizeof(ifr));

        ifr.ifr_flags = IFF_TUN;

        if(!devname.empty())
            strncpy(ifr.ifr_name, devname.c_str(), IFNAMSIZ);

        if( (ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 )
        {
            close(fd);
            throw ErrException("TunDev","Unable to create device");
        }

        this->devname.assign(ifr.ifr_name);
    }

    Tundev::~Tundev() {
        if (fd >= 0) {
            ::close(fd);
            fd = -1;
        }
    }

    void Tundev::setMTU(int mtu) {
        int s = socket(PF_INET, SOCK_DGRAM, 0);
        struct ifreq ifr;
        ifr.ifr_mtu = mtu;
        strncpy(ifr.ifr_name, devname.c_str(), sizeof(ifr.ifr_name));
        if( (ioctl(s, SIOCSIFMTU, &ifr)) < 0 )
        {
            close(s);
            throw ErrException("TunDev","Unable to set MTU");
        }
        close(s);
    }
}
