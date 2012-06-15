/**
 * @file udpsocket.h
 * @brief UDP socket abstraction layer
 */
#ifndef IOM_NET_UDPSOCKET_H
#define IOM_NET_UDPSOCKET_H

#include "socket.h"

namespace iom
{
    class UDPServer;

    /**
     * @class UDPSocket
     * @brief UDP socket abstraction layer
     */
    class UDPSocket : public Socket
    {
    public:
        /**
         * @brief Initialise an empty TCP socket
         */
        UDPSocket();
        /**
         * @brief Initialise a TCP socket with a remote address
         */
        UDPSocket(const SockAddress& addr);
        /**
         * @brief Build a new Socket object from an existing connection
         */
        UDPSocket(int sock, const SockAddress &addr);

        /**
         * @brief Send data through the socket
         *
         * @param data sent data
         * @param size size of data to be sent
         * @return number of sent bytes
         */
        int send(const void *data, int size);

        /**
         * @brief Receive data from the socket
         *
         * @param data received data buffer
         * @param size size of data buffer
         * @return number of received bytes
         */
        int recv(void *data, int size);

        /**
         * @brief Get the remote address from which the last packet was received
         * @return remote address
         */
        SockAddress getLastRecvAddr() const;

    private:
        SockAddress lastRecvAddr;
    };

}
#endif /* IOM_NET_UDPSOCKET_H */
