#include "ipovermesh.h"

using namespace iom;

int main() {
    try {
        log::init();
        log::info << "start" << log::endl;

        // Test addresses
        SockAddress a("10.42.0.1:4545");
        Address a2("::1");
        //log::debug << "Addr " << a << " " << a2 << " (" << a2.toString() << ")" << log::endl;

        // Test iface listing
        //std::vector<NetIf> ifaces = NetIf::getAllNet();
        std::vector<NetIf> ifaces = NetIf::getWifiUp();
        log::info << "Active Wifi interfaces:" << log::endl;
        for (std::vector<NetIf>::const_iterator i = ifaces.begin(); i != ifaces.end(); i++) {
            log::info << "- " << *i << log::endl;
        }

        Tundev tun;
        tun.setMTU(1300);
        system("ifconfig -a");
    } catch (Exception e) {
        log::fatal << e << log::endl;
    }

    log::flushAndQuit();
    return 0;
}
