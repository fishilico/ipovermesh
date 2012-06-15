/**
 * @file sockpacket.h
 * @brief Packet with a socket-sending ability
 */

#ifndef IOM_NET_SOCKPACKET_H
#define IOM_NET_SOCKPACKET_H

#include "socket.h"

namespace iom
{
    class SockPacket
    {
    public:
        /**
         * @brief Build the raw text query for this N2NP Packet
         * @param newData newly-allocated buffer with ready-to-send data
         * @return size of *data
         *
         * Note: Please delete *newData with delete[]
         */
        virtual unsigned long build(char **newData) const = 0;

        /**
         * @brief Send packet with a Socket
         * @param sock
         * @return true if it succeeded
         */
        virtual bool send(Socket& sock) const;
    };
}

#endif /* IOM_NET_SOCKPACKET_H */

