#include "rreplypacket.h"
#include "../core/common.h"
#include <boost/algorithm/string.hpp>

namespace iom
{

    RReplyPacket::RReplyPacket(const Address& source, const Address& destination,
        const Address& nexthop, unsigned int n)
    :source(source), destination(destination), nexthop(nexthop), n(n) {
    }

    RReplyPacket::RReplyPacket(const GTTPacket& gttpkt) {
        std::map<std::string, std::string>::const_iterator it;

        // Check protocol
        if (gttpkt.protocol.compare("MESH")) {
            log::error << "RReply: Invalid GTT protocol "
                << gttpkt.protocol << log::endl;
            throw FailException("RReplyPacket", "Invalid GTT packet");
        }
        if (gttpkt.method.compare("RREP")) {
            log::error << "RReply: Invalid GTT method "
                << gttpkt.method << log::endl;
            throw FailException("RReplyPacket", "Invalid GTT packet");
        }

        // Parse headers
        for (it = gttpkt.headers.begin(); it != gttpkt.headers.end(); it++) {
            if (boost::iequals(it->first, "Source")) {
                source = Address(it->second);
            } else if (boost::iequals(it->first, "Destination")) {
                destination = Address(it->second);
            } else if (boost::iequals(it->first, "NextHop")) {
                nexthop = Address(it->second);
            } else if (boost::iequals(it->first, "N")) {
                n = String::toInt(it->second);
            } else {
                log::error << "RReply: Unknown header "
                    << it->first << log::endl;
                throw MinorException("RReplyPacket", "Invalid GTT header");
            }
        }
    }

    RReplyPacket::RReplyPacket(const RReplyPacket& pkt)
    :source(pkt.source), destination(pkt.destination), nexthop(pkt.nexthop), n(pkt.n) {
    }

    const RReplyPacket& RReplyPacket::operator=(const RReplyPacket& pkt) {
        if (this != &pkt) {
            this->source = pkt.source;
            this->destination = pkt.destination;
            this->nexthop = pkt.nexthop;
            this->n = pkt.n;
        }
        return *this;
    }

    unsigned long RReplyPacket::build(char **newData) const {
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
        gttpkt.headers["NextHop"] = nexthop.toString();
        gttpkt.headers["N"] = String::fromInt(n);
    }
}
