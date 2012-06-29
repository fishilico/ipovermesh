#include "udpsocket.h"
#include "../core/common.h"

namespace iom
{

    UDPSocket::UDPSocket() {
    }

    UDPSocket::UDPSocket(const SockAddress& addr)
    :Socket(addr) {
    }

    UDPSocket::UDPSocket(int sock, const SockAddress &addr)
    :Socket(sock, addr) {
    }

    int UDPSocket::send(const void *data, int size) {
        int bytes;
        struct sockaddr_storage saddr;

        BOOST_ASSERT(data != NULL);
        // Create a new socket if it does not exist
        if (this->sock < 0) {
            this->sock = ::socket(AF_INET, SOCK_DGRAM, 0);
            if (this->sock == -1)
                throw ErrException("UDPSocket", "socket");
        }
        address.getSockAddr((struct sockaddr *) &saddr);
        bytes = ::sendto(this->sock, data, size, 0, (const struct sockaddr *) &saddr, sizeof (saddr));
        if (localAddress.empty())
            this->updateLocalAddress();
        // TODO: Implement status error (ex. Conn closed, ...)

        if (bytes < 0)
            throw ErrException("UDPSocket", "sendto");
        return bytes;
    }

    int UDPSocket::recv(void * data, int size) {
        int bytes;
        struct sockaddr_storage saddr;
        socklen_t length = sizeof (saddr);
        BOOST_ASSERT(data != NULL);
        BOOST_ASSERT(this->sock >= 0);
        bytes = ::recvfrom(this->sock, data, size, 0, (struct sockaddr *) &saddr, &length);
        // TODO: Implement status error (eg. Conn closed, ...)
        if (bytes < 0)
            throw ErrException("UDPSocket", "recvfrom");

        if (bytes == 0) {
            //Socket is now closed
            BOOST_VERIFY(sock < 0);
            return 0;
        }

        /**
         * recv doesn't set the after-the-last byte to zero. We must do it to
         * avoid some issues.
         * (writing into a prefilled longer data buffer fucks everything up)
         */
        if (bytes < size)
            ((char*) data)[bytes] = 0;
        lastRecvAddr = SockAddress((const sockaddr *) &saddr);
        return bytes;
    }

    SockAddress UDPSocket::getLastRecvAddr() const {
        return lastRecvAddr;
    }

}
