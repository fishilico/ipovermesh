#include "ctlsocket.h"
#include "../core/common.h"
#include <cstring>
#include <sys/ioctl.h>

namespace iom
{

    CtlSocket::CtlSocket(const std::string& devname, int ipVersion)
    :devname(devname) {
        int family = 0;
        switch (ipVersion) {
            case 4:
                family = PF_INET;
                break;
            case 6:
                family = PF_INET;
                break;
            default:
                throw FailException("CtlSocket", "Bad IP version");
        }
        sock = ::socket(family, SOCK_DGRAM, 0);
        if (sock < 0)
            throw ErrException("CtlSocket", "Unable to open a socket");
    }

    CtlSocket::~CtlSocket() {
        if (sock >= 0)
            ::close(sock);
    }

    bool CtlSocket::setMTU(int mtu) {
        struct ifreq ifr;
        ifr.ifr_mtu = mtu;
        return this->ioctlReq(SIOCSIFMTU, ifr);
    }

    int CtlSocket::getMTU() {
        struct ifreq ifr;
        if (!this->ioctlReq(SIOCGIFMTU, ifr))
            return 0;
        return ifr.ifr_mtu;
    }

    bool CtlSocket::getHwAddr(unsigned char hwaddr[6]) {
        // ARPHRD_ETHER family
        struct ifreq ifr;
        if (!this->ioctlReq(SIOCGIFHWADDR, ifr))
            return false;
        memcpy(hwaddr, ifr.ifr_hwaddr.sa_data, 6);
        return true;
    }

    bool CtlSocket::activateInterface() {
        struct ifreq ifr;
        if (!ioctlReq(SIOCGIFFLAGS, ifr))
            return false;
        ifr.ifr_flags = ifr.ifr_flags | IFF_UP;
        return (ioctlReq(SIOCSIFFLAGS, ifr));
    }

    bool CtlSocket::ioctlReq(int request, struct ifreq& ifr) {
        BOOST_ASSERT(sock >= 0);
        strncpy(ifr.ifr_name, devname.c_str(), sizeof (ifr.ifr_name));
        if ((::ioctl(sock, request, &ifr)) < 0) {
            log::error << "Socket IO Ctl error: " << log::errstd << log::endl;
            return false;
        }
        return true;
    }
}
