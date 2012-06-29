/**
 * @file server.h
 * @brief Basic Internet server class
 */
#ifndef IOM_NET_SERVER_H
#define IOM_NET_SERVER_H

#include "sockaddress.h"
#include <boost/noncopyable.hpp>

namespace iom
{
    /**
     * @class Server
     * @brief Server interface
     */
    class Server : private boost::noncopyable
    {
    public:
        /**
         * @brief Create a new generic unbinded server
         * @param addr
         */
        Server(const SockAddress& addr);
        virtual ~Server();

        /**
         * @brief Stop the server (shutdown + close)
         */
        virtual void close();

        /**
         * @brief Tell wether server is binded or not
         * @return true if server is binded
         */
        bool isBinded() const;

        /**
         * @brief get server address
         * @return aserver Address object
         */
        const SockAddress& getAddress() const;


        /**
         * @brief Return internal file descriptor
         */
        int getFd() const;

    protected:
        /**
         * @brief Server binded address
         */
        SockAddress address;
        /**
         * @brief Server socket file descriptor
         */
        int sockfd;

        /**
         * @brief Common code for TCP and UDP bind
         * @param socktype: SOCK_STREAM (TCP) or SOCK_DGRAM (UDP)
         * @return true on success
         */
        bool _internal_bind(int socktype);
    };

}
#endif /* IOM_NET_SERVER_H */
