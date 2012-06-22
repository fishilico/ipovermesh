#include "rreplypacket.h"
#include "../core/common.h"
#include <boost/algorithm/string.hpp>

namespace iom
{

    RReplyPacket::RReplyPacket(const Address& source, const Address& destination,
        const Address& sender, const Address& nexthop, unsigned int n)
    :source(source), destination(destination), sender(sender), nexthop(nexthop), n(n) {
    }

    RReplyPacket::RReplyPacket(const GTTPacket& gttpkt) {
        std::map<std::string, std::string>::const_iterator it;

        // Check protocol
        if (gttpkt.protocol.compare("MESH")) {
            log::error << "RReply: Invalid GTT protocol "
                << gttpkt.protocol << log::endl;
            throw ParserException("RReplyPacket", "Invalid GTT packet");
        }
        if (gttpkt.method.compare("RREP")) {
            log::error << "RReply: Invalid GTT method "
                << gttpkt.method << log::endl;
            throw ParserException("RReplyPacket", "Invalid GTT packet");
        }

        // Parse headers
        for (it = gttpkt.headers.begin(); it != gttpkt.headers.end(); it++) {
            if (boost::iequals(it->first, "Source")) {
                source = Address(it->second);
            } else if (boost::iequals(it->first, "Destination")) {
                destination = Address(it->second);
            } else if (boost::iequals(it->first, "Sender")) {
                sender = Address(it->second);
            } else if (boost::iequals(it->first, "NextHop")) {
                nexthop = Address(it->second);
            } else if (boost::iequals(it->first, "N")) {
                n = String::toInt(it->second);
            } else {
                log::error << "RReply: Unknown header "
                    << it->first << log::endl;
                throw ParserException("RReplyPacket", "Invalid GTT header");
            }
        }
    }

    unsigned long RReplyPacket::build(unsigned char **newData) const {
        BOOST_ASSERT(newData != NULL);
        // Build a GTT Packet to build
        GTTPacket gttpkt;
        this->fillGTTPacket(gttpkt);
        return gttpkt.build(newData);
    }

    void RReplyPacket::fillGTTPacket(GTTPacket &gttpkt) const {
        gttpkt.protocol = "MESH";
        gttpkt.method = "RREP";
        gttpkt.headers["Source"] = source.toString();
        gttpkt.headers["Destination"] = destination.toString();
        gttpkt.headers["Sender"] = sender.toString();
        gttpkt.headers["NextHop"] = nexthop.toString();
        gttpkt.headers["N"] = String::fromInt(n);
    }
}
