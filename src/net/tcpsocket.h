/**
 * @file tcpsocket.h
 * @brief TCP socket abstraction layer
 */
#ifndef IOM_NET_TCPSOCKET_H
#define IOM_NET_TCPSOCKET_H

#include "socket.h"

namespace iom
{

    /**
     * @class TCPSocket
     *
     * @brief TCP socket abstraction layer
     */
    class TCPSocket : public Socket
    {
    public:
        /**
         * @brief Initialise an empty TCP socket
         */
        TCPSocket();
        /**
         * @brief Initialise a TCP socket with a remote address
         */
        TCPSocket(const SockAddress& addr);
        /**
         * @brief Build a new Socket object from an existing connection
         */
        TCPSocket(int sock, const SockAddress &addr);

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
    private:
        bool connect();
        bool isConnected;
    };

}
#endif /* IOM_NET_UDPSOCKET_H */
