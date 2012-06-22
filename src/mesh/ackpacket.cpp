#include "ackpacket.h"
#include "../core/common.h"
#include <boost/algorithm/string.hpp>

namespace iom {

    AckPacket::AckPacket(const Address& source, const Address& destination, const Address& nexthop, unsigned int seq) :
        source(source), destination(destination), nexthop(nexthop), seq(seq)
    {
    }

    AckPacket::AckPacket(const GTTPacket& gttpkt) {
        std::map<std::string, std::string>::const_iterator it;

        // Check protocol
        if (gttpkt.protocol.compare("MESH")) {
            log::error << "ACK: Invalid GTT protocol "
                << gttpkt.protocol << log::endl;
            throw FailException("AckPacket", "Invalid GTT packet");
        }
        if (gttpkt.method.compare("ACK")) {
            log::error << "ACK: Invalid GTT method "
                << gttpkt.method << log::endl;
            throw FailException("AckPacket", "Invalid GTT packet");
        }

        // Parse headers
        for (it = gttpkt.headers.begin(); it != gttpkt.headers.end(); it++) {
            if (boost::iequals(it->first, "Source")) {
                source = Address(it->second);
            } else if (boost::iequals(it->first, "Destination")) {
                destination = Address(it->second);
            } else if (boost::iequals(it->first, "NextHop")) {
                nexthop = Address(it->second);
            } else if (boost::iequals(it->first, "Seq")) {
                seq = String::toInt(it->second);
            } else {
                log::error << "ACK: Unknown header "
                    << it->first << log::endl;
                throw MinorException("AckPacket", "Invalid GTT header");
            }
        }
    }

    unsigned long AckPacket::build(char **newData) const {
        BOOST_ASSERT(newData != NULL);
        // Build a GTT Packet to build
        GTTPacket gttpkt;
        this->fillGTTPacket(gttpkt);
        return gttpkt.build(newData);
    }

    void AckPacket::fillGTTPacket(GTTPacket &gttpkt) const {
        gttpkt.protocol = "MESH";
        gttpkt.method = "ACK";
        gttpkt.headers["Source"] = source.toString();
        gttpkt.headers["Destination"] = destination.toString();
        gttpkt.headers["NextHop"] = nexthop.toString();
        gttpkt.headers["Seq"] = String::fromInt(seq);
    }
}
