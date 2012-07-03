/**
 * @file udpserver.h
 * @brief UDP server abstraction layer
 */
#ifndef IOM_NET_UDPSERVER_H
#define IOM_NET_UDPSERVER_H

#include "server.h"
#include "udpsocket.h"

namespace iom
{
    /**
     * @class UDPServer
     * @brief UDP server abstraction layer
     */
    class UDPServer : public Server
    {
    public:
        /**
         * @brief Create a new UDP server
         * @param addr IP address and UDP port to bind to
         */
        UDPServer(const SockAddress& addr);

        virtual void close();

        /**
         * @brief Receive data for the server
         *
         * @param data received data buffer
         * @param size size of data buffer
         * @return number of received bytes
         */
        int recv(void *data, int size);

        // send is not implemented yet
        //int send(const void *data, int size);

        /**
         * @brief Bind socket to a specific device
         * @param devicename
         */
        void bindToDevice(const std::string& devicename);

    private:
        UDPSocket sock;
    };
}
#endif /* IOM_NET_UDPSERVER_H */

