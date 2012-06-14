/**
 * @file sockaddress.h
 * @brief A socket address is an (IP, port) tuple
 */
#ifndef IOM_NET_ADDRESS_H
#define IOM_NET_ADDRESS_H

#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <vector>

namespace iom
{
    /**
     * @class SockAddress
     *
     * @brief Manage IP:port association
     *
     * This class manage both IPv4 et IPv6 address
     */
    class SockAddress
    {
    public:
        /**
         * @brief Empty constructor
         */
        SockAddress();
        /**
         * @brief Create an address with IP and port
         * @param ip IP address
         * @param port TCP or UDP port
         * @param ipVersion 4 or 6
         */
        SockAddress(const std::string& ip, unsigned short port, int ipVersion = 4);
        /**
         * @brief Create an address from a sockaddr
         * @param saddr input sockaddr
         */
        SockAddress(const struct sockaddr *saddr);
        /**
         * @brief Copy constructor
         */
        SockAddress(const SockAddress& addr);
        /**
         * @brief Address from ip:port string
         **/
        SockAddress(const std::string& addressFormat);

        /**
         * @brief Assignment constructor
         */
        SockAddress& operator=(const SockAddress& addr);

        /**
         * @brief Get IP address
         * @return ip
         */
        const std::string& getIp() const;
        /**
         * @brief Get port
         * @return port
         */
        unsigned short getPort() const;

        /**
         * @brief Is it empty ?
         * @return the answer
         */
        bool empty() const;

        /**
         * @brief Create a sockaddr structure from the address
         * @param saddr sockaddr structure
         *
         * @note Use a sockaddr_storage stucture to get information for IPv4
         * and IPv6 addresses
         */
        void getSockAddr(struct sockaddr *saddr) const;

        /**
         * @brief Print the address in an output stream
         * @param os output stream
         * @param addr
         */
        friend std::ostream& operator<<(std::ostream& os, const SockAddress& addr);

        /**
         * @brief get a std::string representation of the address
         */
        std::string toString() const;

        /**
         * @brief compare this and addr
         * @param addr
         */
        int compare(const SockAddress& addr) const;

        /**
         * @brief Equality test
         * @param addr1
         * @param addr2
         */
        friend bool operator==(const SockAddress& addr1, const SockAddress& addr2);

        /**
         * @brief Inequality test
         * @param addr1
         * @param addr2
         */
        friend bool operator!=(const SockAddress& addr1, const SockAddress& addr2);

        /**
         * @brief Less test
         * @param addr1
         * @param addr2
         */
        friend bool operator<(const SockAddress& addr1, const SockAddress& addr2);

        /**
         * @brief Get interface addresses
         * @param port
         * @return a std::vector of addresses
         */
        static std::vector<SockAddress> getIfaceAdresses(int port = 0);

    private:
        // IP string
        std::string ip;
        // TCP or UDP port
        unsigned short port;
        // 4 or 6
        int ipVersion;

    };
}

#endif /* IOM_NET_ADDRESS_H */
