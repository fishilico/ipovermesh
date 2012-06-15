#include "tundev.h"
#include "ctlsocket.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <arpa/inet.h>
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
        this->mtu = mtu;
        CtlSocket s(devname);
        if (!s.setMTU(mtu))
            throw FailException("TunDev", "Unable to set MTU");
    }

    void Tundev::setIPv6(const Address &ip) {
        std::string cmd = "ifconfig ";
        cmd += devname;
        cmd += " add ";
        cmd += ip.toString();
        if(system(cmd.c_str()) != 0)
            throw FailException("TunDev", "Unable to set IP");
    }

    void Tundev::activate() {
        CtlSocket s(devname);
        if (!s.activateInterface())
            throw FailException("TunDev", "Device could not be activated");
    }

    int Tundev::read(void *__buf, size_t __nbytes) {
        return ::read(fd, __buf, __nbytes);
    }

    IPv6Packet Tundev::readPacket() {
        unsigned char data[mtu];
        int size = read(data, mtu);
        return IPv6Packet(data, size);
    }

    int Tundev::write(const void *__buf, size_t __nbytes) {
        return ::write(fd, __buf, __nbytes);
    }

    bool Tundev::writePacket(const IPv6Packet &data) {
        return write(data.getData(), data.getSize()) == data.getSize();
    }
}
