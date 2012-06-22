#include "pktpacket.h"
#include "../core/common.h"
#include <boost/algorithm/string.hpp>

namespace iom
{

    PktPacket::PktPacket(const Address& source, const Address& destination,
        const Address& nexthop, unsigned int seq,
        unsigned long size, const boost::shared_array<unsigned char>& data)
    :source(source), destination(destination), nexthop(nexthop), seq(seq),
    size(size), data(data) {
    }

    PktPacket::PktPacket(const GTTPacket& gttpkt) {
        std::map<std::string, std::string>::const_iterator it;

        // Check protocol
        if (gttpkt.protocol.compare("MESH")) {
            log::error << "RReply: Invalid GTT protocol "
                << gttpkt.protocol << log::endl;
            throw ParserException("RReplyPacket", "Invalid GTT packet");
        }
        if (gttpkt.method.compare("PKT")) {
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
            } else if (boost::iequals(it->first, "NextHop")) {
                nexthop = Address(it->second);
            } else if (boost::iequals(it->first, "Seq")) {
                seq = String::toInt(it->second);
            } else {
                log::error << "RReply: Unknown header "
                    << it->first << log::endl;
                throw ParserException("PktPacket", "Invalid GTT header");
            }
        }

        // Copy data
        size = gttpkt.size;
        data = gttpkt.body;
    }

    PktPacket::PktPacket(const IPv6Packet& ippkt, const Address& nexthop, unsigned int seq)
    :source(ippkt.getSourceAddress()),
    destination(ippkt.getDestinationAddress()),
    nexthop(nexthop), seq(seq), size(ippkt.getSize()), data(ippkt.getData()) {
    }

    unsigned long PktPacket::build(unsigned char **newData) const {
        BOOST_ASSERT(newData != NULL);
        // Build a GTT Packet to build
        GTTPacket gttpkt(size, data);
        gttpkt.protocol = "MESH";
        gttpkt.method = "PKT";
        gttpkt.headers["Source"] = source.toString();
        gttpkt.headers["Destination"] = destination.toString();
        gttpkt.headers["NextHop"] = nexthop.toString();
        gttpkt.headers["Seq"] = String::fromInt(seq);
        return gttpkt.build(newData);
    }
}
