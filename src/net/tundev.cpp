#include "tundev.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
//#include <linux/if.h>
//#include <linux/if_tun.h>

namespace iom
{

    Tundev::Tundev(const std::string& devname) {
        fd = ::open("/dev/net/tun", O_RDWR);
        if (fd < 0)
            throw ErrException("TunDev", "Unable to open /dev/net/tun");
    }

    Tundev::~Tundev() {
        if (fd >= 0) {
            ::close(fd);
            fd = -1;
        }
    }
}
