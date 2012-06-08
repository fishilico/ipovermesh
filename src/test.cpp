#include "ipovermesh.h"

using namespace iom;

int main() {
    try {
        log::init();
        log::info << "start" << log::endl;
    } catch (Exception e) {
        log::fatal << e << log::endl;
    }
    log::flushAndQuit();
    return 0;
}
