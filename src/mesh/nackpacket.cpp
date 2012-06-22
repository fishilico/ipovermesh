#include "nackpacket.h"
#include "../core/common.h"
#include <boost/algorithm/string.hpp>

namespace iom {

    NAckPacket::NAckPacket(const Address& source, const Address& destination, const Address& nexthop) :
        source(source), destination(destination), nexthop(nexthop)
    {
    }

    NAckPacket::NAckPacket(const GTTPacket& gttpkt) {
        std::map<std::string, std::string>::const_iterator it;

        // Check protocol
        if (gttpkt.protocol.compare("MESH")) {
            log::error << "NACK: Invalid GTT protocol "
                << gttpkt.protocol << log::endl;
            throw FailException("NAckPacket", "Invalid GTT packet");
        }
        if (gttpkt.method.compare("NACK")) {
            log::error << "NACK: Invalid GTT method "
                << gttpkt.method << log::endl;
            throw FailException("NAckPacket", "Invalid GTT packet");
        }

        // Parse headers
        for (it = gttpkt.headers.begin(); it != gttpkt.headers.end(); it++) {
            if (boost::iequals(it->first, "Source")) {
                source = Address(it->second);
            } else if (boost::iequals(it->first, "Destination")) {
                destination = Address(it->second);
            } else if (boost::iequals(it->first, "NextHop")) {
                nexthop = Address(it->second);
            } else {
                log::error << "NACK: Unknown header "
                    << it->first << log::endl;
                throw MinorException("NAckPacket", "Invalid GTT header");
            }
        }
    }

    unsigned long NAckPacket::build(unsigned char **newData) const {
        BOOST_ASSERT(newData != NULL);
        // Build a GTT Packet to build
        GTTPacket gttpkt;
        this->fillGTTPacket(gttpkt);
        return gttpkt.build(newData);
    }

    void NAckPacket::fillGTTPacket(GTTPacket &gttpkt) const {
        gttpkt.protocol = "MESH";
        gttpkt.method = "NACK";
        gttpkt.headers["Source"] = source.toString();
        gttpkt.headers["Destination"] = destination.toString();
        gttpkt.headers["NextHop"] = nexthop.toString();
    }
}
