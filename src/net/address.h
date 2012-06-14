/**
 * @file address.h
 * @brief IP address
 */

#ifndef IOM_NET_ADDRESS_H
#define IOM_NET_ADDRESS_H

#include <iostream>
#include <string>
#include <arpa/inet.h>

namespace iom
{
    /**
     * @class Address
     *
     * @brief Manage IP address
     *
     * This class manage both IPv4 et IPv6 address
     */
    class Address
    {
    public:
        Address();
        Address(const std::string& ip, int ipVersion = 0);
        Address(const struct sockaddr *saddr);

        /**
         * @brief Copy constructor
         */
        Address(const Address& addr);

        /**
         * @brief Assignment constructor
         */
        Address& operator=(const Address& addr);

        /**
         * @brief Fill in a sockaddr struct, without the port
         * @param saddr
         * @param port (optional) transport port
         */
        void getSockAddr(struct sockaddr *saddr, unsigned short port = 0) const;

        /**
         * @return IP version
         */
        int getVersion() const;

        /**
         * @brief Is it empty ?
         * @return the answer
         */
        bool empty() const;

        /**
         * @brief Print the address in an output stream
         * @param os output stream
         * @param addr
         */
        friend std::ostream& operator<<(std::ostream& os, const Address& addr);

        /**
         * @brief get a std::string representation of the IP address
         */
        std::string toString(bool addBrackets = false) const;

        /**
         * @brief compare this and addr
         * @param addr
         */
        int compare(const Address& addr) const;

        /**
         * @brief Equality test
         * @param addr1
         * @param addr2
         */
        friend bool operator==(const Address& addr1, const Address& addr2);

        /**
         * @brief Inequality test
         * @param addr1
         * @param addr2
         */
        friend bool operator!=(const Address& addr1, const Address& addr2);

        /**
         * @brief Less test
         * @param addr1
         * @param addr2
         */
        friend bool operator<(const Address& addr1, const Address& addr2);

    private:
        std::string ip;
        // 4 or 6
        int ipVersion;
    };
}

#endif /* IOM_NET_ADDRESS_H */

