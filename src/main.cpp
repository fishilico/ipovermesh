/**
 * @file main.cpp
 * 
 * Main file for IP over MESH project
 */

#include "ipovermesh.h"

using namespace iom;

class IpOverMesh : private boost::noncopyable {
public:
    IpOverMesh();
    ~IpOverMesh();
    void stop();
    
    void tun2wifiLoop();
    void wifi2tunLoop();
    
private:
    Wifi wifi;
    Tundev tun;
    
    boost::thread *tun2wifi;
    boost::thread *wifi2tun;
    
    bool isRunning;
};

IpOverMesh::IpOverMesh() {
    // Start wifi interface
    const NetIf iface = wifi.getInterface();
    const Address tunaddr = wifi.getAddress();
    log::info << "WiFi started on " << iface.getName() << ", "
        << iface.getAddress() << log::endl;

    // Start Tun
    log::info << "Starting Tun for " << tunaddr << "..." << log::endl;
    tun.setMTU(1300);
    tun.setIPv6(tunaddr);
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
                log::debug << "Transmit " << p->getSize() << " bytes "
                    << p->getSourceAddress() << " -> " << p->getDestinationAddress()
                    << log::endl;
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
                log::debug << "Transmit " << p->getSize() << " bytes"
                    << p->getSourceAddress() << " -> " << p->getDestinationAddress()
                    << log::endl;
                tun.writePacket(*p);
            }
        }
        log::debug << "Thread ending" << log::endl;
    } catch (Exception e) {
        log::fatal << e << log::endl;
    }
}

int main() {
    try {
        log::init();
        IpOverMesh self;
        system("ifconfig -a");
        
        log::info << "Press enter to stop the program" << log::endl;
        unsigned char c;
        read(0, &c, 1);
    } catch (Exception e) {
        log::fatal << e << log::endl;
    }

    log::flushAndQuit();
    return 0;
}
