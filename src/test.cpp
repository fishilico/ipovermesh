#include "ipovermesh.h"

using namespace iom;

int main() {
    try {
        log::init();
        log::info << "start" << log::endl;
        Tundev tun;
        tun.setMTU(1300);
        system("ifconfig -a");
    } catch (Exception e) {
        log::fatal << e << log::endl;
    }

    log::flushAndQuit();
    return 0;
}
