#include "ipovermesh.h"

using namespace iom;

int main() {
    try {
        log::init();
        log::info << "start" << log::endl;
        SockAddress a("10.42.0.1:4545");
        Address a2("::1");
        log::debug << "Addr " << a << " " << a2 << " (" << a2.toString() << ")" << log::endl;
        Tundev tun;
        tun.setMTU(1300);
        system("ifconfig -a");
    } catch (Exception e) {
        log::fatal << e << log::endl;
    }

    log::flushAndQuit();
    return 0;
}
