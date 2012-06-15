#include "rreqpacket.h"
#include "../core/common.h"
#include <boost/algorithm/string.hpp>

namespace iom
{

    RReqPacket::RReqPacket(const Address& source, const Address& destination, unsigned int n)
    :source(source), destination(destination), n(n) {
    }

    RReqPacket::RReqPacket(const GTTPacket& gttpkt) {
        std::map<std::string, std::string>::const_iterator it;

        // Check protocol
        if (gttpkt.protocol.compare("RREQ")) {
            log::error << "RReq: Invalid GTT protocol "
                << gttpkt.protocol << log::endl;
            throw FailException("RReqPacket", "Invalid GTT packet");
        }

        // Parse headers
        for (it = gttpkt.headers.begin(); it != gttpkt.headers.end(); it++) {
            if (boost::iequals(it->first, "Source")) {
                source = Address(it->second);
            } else if (boost::iequals(it->first, "Destination")) {
                destination = Address(it->second);
            } else if (boost::iequals(it->first, "N")) {
                n = String::toInt(it->second);
            } else {
                log::error << "RReqPacket: Unknown header "
                    << it->first << log::endl;
                throw MinorException("RReqPacket", "Invalid GTT header");
            }
        }
    }

    RReqPacket::RReqPacket(const RReqPacket& pkt)
    :source(pkt.source), destination(pkt.destination), n(pkt.n) {
    }

    const RReqPacket& RReqPacket::operator=(const RReqPacket& pkt) {
        if (this != &pkt) {
            this->source = pkt.source;
            this->destination = pkt.destination;
            this->n = pkt.n;
        }
        return *this;
    }

    unsigned long RReqPacket::build(char **newData) const {
        BOOST_ASSERT(newData != NULL);
        // Build a GTT Packet to build
        GTTPacket gttpkt;
        this->fillGTTPacket(gttpkt);
        return gttpkt.build(newData);
    }

    void RReqPacket::fillGTTPacket(GTTPacket &gttpkt) const {
        gttpkt.protocol = "RREQ";
        gttpkt.method = "";
        gttpkt.headers["Source"] = source.toString();
        gttpkt.headers["Destination"] = destination.toString();
        gttpkt.headers["N"] = String::fromInt(n);
    }
}
