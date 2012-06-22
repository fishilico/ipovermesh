#include "ipovermesh.h"
#include "parser/gttparser.h"

using namespace iom;

void test_pkt() {
    const Address a1("42.13.37.1");
    const Address a2("42.13.37.2");
    const Address a3("42.13.37.3");
    char *data = NULL;
    unsigned long size;

    // Reply
    RReplyPacket rrep(a1, a2, a1, a3, 42);
    size = rrep.build(&data);
    BOOST_VERIFY(data != NULL);
    log::debug << "RReply:\n" << data << log::endl;

    // Read reply
    GTTParser parser;
    GTTPacket *gttpkt;
    parser.eat(data, size);
    while ((gttpkt = parser.getPacket()) != NULL) {
        RReplyPacket rrep2(*gttpkt);
        log::debug << "RReply: " << rrep2.destination << log::endl;
    }
    delete[] data;
}

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

        test_pkt();

        Tundev tun;
        tun.setMTU(1300);
        tun.setIPv6(Address("aaaa::2"));
        tun.activate();
        system("ifconfig -a");
        while (true) {
            IPv6Packet p = tun.readPacket();
            log::info << p.getSourceAddress() << "->" << p.getDestinationAddress() << log::endl;
            Address source = p.getSourceAddress();
            p.setSourceAddress(p.getDestinationAddress());
            p.setDestinationAddress(source);
            tun.writePacket(p);
        }
    } catch (Exception e) {
        log::fatal << e << log::endl;
    }

    log::flushAndQuit();
    return 0;
}
