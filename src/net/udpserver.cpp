#include "udpserver.h"
#include "../core/common.h"

namespace iom
{

    UDPServer::UDPServer(const SockAddress& addr)
    :Server(addr) {
        if (!this->_internal_bind(SOCK_DGRAM))
            return;

        // Directly create a socket to receive connections
        sock.setFd(sockfd);
        sock.setLocalAddress(address);
    }

    void UDPServer::close() {
        sockfd = -1;
        sock.close();
    }

    int UDPServer::recv(void *data, int size) {
        int r = sock.recv(data, size);
        //sock.setAddress(sock.getLastRecvAddr());// This is not thread-safe
        return r;
    }

    /*
    int UDPServer::send(const void *data, int size) {
        throw FailException("UDPServer", "UDPServer::send is not yet implemented");
        return sock.send(data, size);
    }
     */

    void UDPServer::bindToDevice(const std::string& devicename) {
        sock.bindToDevice(devicename);
    }
}
