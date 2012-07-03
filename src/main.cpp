/**
 * @file main.cpp
 *
 * Main file for IP over MESH project
 */

#include "ipovermesh.h"

// Tun -> MESH (input) in yellow
#define TTY_COLOR_INPUT "\033[33;1m"
// MESH -> Tun (output) in green
#define TTY_COLOR_OUTPUT "\033[32;1m"
#define TTY_COLOR_NORM "\033[m"

using namespace iom;

class IpOverMesh : private boost::noncopyable {
public:
    IpOverMesh(const NetIf& wifiIface);
    ~IpOverMesh();
    void stop();

    void tun2wifiLoop();
    void wifi2tunLoop();

    const Address& getTunAddress() const;

private:
    Wifi wifi;
    Tundev tun;

    boost::thread *tun2wifi;
    boost::thread *wifi2tun;

    bool isRunning;
};

IpOverMesh::IpOverMesh(const NetIf& wifiIface)
:wifi(wifiIface)
{
    // Start wifi interface
    const Address tunaddr = wifi.getAddress();
    log::info << "WiFi started on " << wifiIface.getName() << ", "
        << wifiIface.getAddress() << log::endl;

    // Start Tun
    log::info << "Starting Tun for " << tunaddr << "..." << log::endl;
    tun.setMTU(1300);
    tun.setIPv6(tunaddr, Wifi::IPprefixLen);
    tun.activate(true);

    isRunning = true;
    tun2wifi = new boost::thread(boost::bind(&IpOverMesh::tun2wifiLoop, this));
    wifi2tun = new boost::thread(boost::bind(&IpOverMesh::wifi2tunLoop, this));
}

IpOverMesh::~IpOverMesh() {
    stop();
    // Close wifi & tun
    log::debug << "Closing Tun..." << log::endl;
    tun.close();
    log::debug << "Closing WiFi..." << log::endl;
    wifi.close();
    BOOST_VERIFY(tun2wifi != NULL && wifi2tun != NULL);
    log::debug << "Stop Tun2Wifi..." << log::endl;
    tun2wifi->join();
    log::debug << "Stop Wifi2Tun..." << log::endl;
    wifi2tun->join();
    log::debug << "Done" << log::endl;
}

void IpOverMesh::stop() {
    isRunning = false;
}

void IpOverMesh::tun2wifiLoop() {
    try {
        log::setThreadName("Tun2Wifi");
        while (isRunning) {
            boost::shared_ptr<IPv6Packet> p = tun.readPacket();
            if (p.get() == 0) {
                log::error << "Read nothing" << log::endl;
            } else if (isRunning) {
                log::debug << TTY_COLOR_INPUT
                    << "[Tunnel] Transmit " << p->getSize() << " bytes "
                    << p->getSourceAddress() << " -> " << p->getDestinationAddress()
                    << TTY_COLOR_NORM << log::endl;
                wifi.send(*p);
            }
        }
        log::debug << "Thread ending" << log::endl;
    } catch (Exception e) {
        log::fatal << e << log::endl;
    }
}

void IpOverMesh::wifi2tunLoop() {
    try {
        log::setThreadName("Wifi2Tun");
        while (isRunning) {
            boost::shared_ptr<IPv6Packet> p = wifi.recv();
            if (p.get() == 0) {
                log::error << "Recv nothing" << log::endl;
            } else if (isRunning) {
                log::debug << TTY_COLOR_OUTPUT
                    << "[Tunnel] Transmit " << p->getSize() << " bytes "
                    << p->getSourceAddress() << " -> " << p->getDestinationAddress()
                    << TTY_COLOR_NORM << log::endl;
                tun.writePacket(*p);
            }
        }
        log::debug << "Thread ending" << log::endl;
    } catch (Exception e) {
        log::fatal << e << log::endl;
    }
}

const Address& IpOverMesh::getTunAddress() const {
    return wifi.getAddress();
}

int main() {
    try {
        log::init();
        Socket::init();
        srand(time(NULL));
        boost::shared_ptr<NetIf> wifiIface = Wifi::getWlanIfIp4Up();
        if (wifiIface.get() == 0) {
            throw FailException("Wifi", "No active wifi interface with IPv4 address");
        }
        IpOverMesh self(*wifiIface);
        system("ifconfig -a");

        log::info << "Tun started on " << self.getTunAddress() << "/"
            << Wifi::IPprefixLen << log::endl;
        log::info << "Press enter to stop the program" << log::endl;
        unsigned char c;
        read(0, &c, 1);
    } catch (Exception e) {
        log::fatal << e << log::endl;
    }

    log::flushAndQuit();
    return 0;
}
