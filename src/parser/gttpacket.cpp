#include "gttpacket.h"
#include "../core/common.h"
#include <sstream>
#include <string.h>

namespace iom
{

    GTTPacket::GTTPacket()
    :size(0), body() {
    }

    GTTPacket::GTTPacket(unsigned long size, const boost::shared_array<unsigned char>& body)
    :size(size), body(body) {
    }

    std::ostream& operator<<(std::ostream& os, const GTTPacket& pkt) {
        os << "[Proto " << pkt.protocol << " method " << pkt.method << std::endl;
        for (std::map<std::string, std::string>::const_iterator i = pkt.headers.begin();
            i != pkt.headers.end(); ++i) {
            os << "  " << (*i).first << ": " << (*i).second << std::endl;
        }
        if (pkt.size > 0) {
            os << "  content-length: " << pkt.size << std::endl;
        }
        os << "]";
        return os;
    }

    unsigned long GTTPacket::build(unsigned char **newData) const {
        BOOST_ASSERT(newData != NULL);
        std::stringstream head;
        std::map<std::string, std::string>::const_iterator it;

        // First line
        head << protocol << " " << method << String::crlf;

        // Headers
        for (it = headers.begin(); it != headers.end(); it++) {
            head << it->first << ": " << it->second << String::crlf;
        }
        if (size > 0) {
            head << "content-length: " << size << String::crlf;
        }
        head << String::crlf;

        // Concatenate headers and content
        const std::string str = head.str();
        unsigned long strSize = str.length();
        unsigned char *data = new unsigned char[strSize + size];
        memcpy(data, str.c_str(), strSize);
        if (size > 0) {
            BOOST_VERIFY(body.get() != NULL);
            memcpy(data + strSize, body.get(), size);
        }
        *newData = data;
        return strSize + size;
    }
}
