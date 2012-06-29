#include "ipovermesh.h"
#include "parser/gttparser.h"
#include "net/ctlsocket.h"

using namespace iom;

void test_pkt() {
    const Address a1("42.13.37.1");
    const Address a2("42.13.37.2");
    const Address a3("42.13.37.3");
    unsigned char *data = NULL;
    unsigned long size;

    // Reply
    RReplyPacket rrep(a1, a2, a1, a3, 42);
    size = rrep.build(&data);
    BOOST_VERIFY(data != NULL);
    log::debug << "RReply:\n" << std::string((const char*) data, size) << log::endl;

    // Read reply
    GTTParser parser;
    GTTPacket *gttpkt;
    parser.eat(data, size);
    while ((gttpkt = parser.getPacket()) != NULL) {
        RReplyPacket rrep2(*gttpkt);
        log::debug << "RReply: " << rrep2.destination << log::endl;
    }
    delete[] data;
    data = NULL;

    // ACK
    AckPacket ack(a1, a2, a3, 42);
    size = ack.build(&data);
    BOOST_VERIFY(data != NULL);
    log::debug << "ACK:\n" << std::string((const char*) data, size) << log::endl;

    // Read ACK
    parser.eat(data, size);
    while ((gttpkt = parser.getPacket()) != NULL) {
        AckPacket ack2(*gttpkt);
        log::debug << "ACK: " << ack2.destination << log::endl;
    }
    delete[] data;
    data = NULL;

    // Pkt
    std::string msg("Hello, world !");
    unsigned char *carray = new unsigned char[msg.length()];
    memcpy(carray, msg.c_str(), msg.length());
    PktPacket pkt(a1, a2, a3, 42, msg.length(), boost::shared_array<unsigned char>(carray));
    size = pkt.build(&data);
    BOOST_VERIFY(data != NULL);
    log::debug << "PKT:\n" << std::string((const char*) data, size) << log::endl;
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
        CtlSocket wifi("wlan0");
        unsigned char hwaddr[6];
        if (wifi.getHwAddr(hwaddr)) {
            log::info << "wlan0, TUN IPv6 " << Address::fromHw(hwaddr) << log::endl;
        }

        test_pkt();

        Tundev tun;
        tun.setMTU(1300);
        tun.setIPv6(Address("aaaa::2"));
        tun.activate();
        system("ifconfig -a");
        while (true) {
            boost::shared_ptr<IPv6Packet> p = tun.readPacket();
            if (p.get() != 0) {
                log::info << p->getSourceAddress() << "->" << p->getDestinationAddress() << log::endl;
                Address source = p->getSourceAddress();
                p->setSourceAddress(p->getDestinationAddress());
                p->setDestinationAddress(source);
                tun.writePacket(*p);
            }
        }
    } catch (Exception e) {
        log::fatal << e << log::endl;
    }

    log::flushAndQuit();
    return 0;
}
