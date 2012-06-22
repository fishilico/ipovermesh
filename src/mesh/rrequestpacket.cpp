#include "rrequestpacket.h"
#include "../core/common.h"
#include <boost/algorithm/string.hpp>

namespace iom
{

    RRequestPacket::RRequestPacket(const Address& source, const Address& destination,
        const Address& sender, unsigned int n)
    :source(source), destination(destination), sender(sender), n(n) {
    }

    RRequestPacket::RRequestPacket(const GTTPacket& gttpkt) {
        std::map<std::string, std::string>::const_iterator it;

        // Check protocol
        if (gttpkt.protocol.compare("MESH")) {
            log::error << "RRequest: Invalid GTT protocol "
                << gttpkt.protocol << log::endl;
            throw FailException("RRequestPacket", "Invalid GTT packet");
        }
        if (gttpkt.method.compare("RREP")) {
            log::error << "RRequest: Invalid GTT method "
                << gttpkt.method << log::endl;
            throw FailException("RRequestPacket", "Invalid GTT packet");
        }

        // Parse headers
        for (it = gttpkt.headers.begin(); it != gttpkt.headers.end(); it++) {
            if (boost::iequals(it->first, "Source")) {
                source = Address(it->second);
            } else if (boost::iequals(it->first, "Destination")) {
                destination = Address(it->second);
            } else if (boost::iequals(it->first, "Sender")) {
                sender = Address(it->second);
            } else if (boost::iequals(it->first, "N")) {
                n = String::toInt(it->second);
            } else {
                log::error << "RRequest: Unknown header "
                    << it->first << log::endl;
                throw MinorException("RRequestPacket", "Invalid GTT header");
            }
        }
    }

    unsigned long RRequestPacket::build(char **newData) const {
        BOOST_ASSERT(newData != NULL);
        // Build a GTT Packet to build
        GTTPacket gttpkt;
        this->fillGTTPacket(gttpkt);
        return gttpkt.build(newData);
    }

    void RRequestPacket::fillGTTPacket(GTTPacket &gttpkt) const {
        gttpkt.protocol = "MESH";
        gttpkt.method = "RREQ";
        gttpkt.headers["Source"] = source.toString();
        gttpkt.headers["Destination"] = destination.toString();
        gttpkt.headers["Sender"] = sender.toString();
        gttpkt.headers["N"] = String::fromInt(n);
    }
}
