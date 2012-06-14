#include "tundev.h"
#include "ctlsocket.h"
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
        CtlSocket s(devname);
        if (!s.setMTU(mtu))
            throw FailException("TunDev", "Unable to set MTU");
    }
}
