#include "sockpacket.h"
#include "../core/common.h"

namespace iom
{

    bool SockPacket::send(Socket& sock) const {
        // Build data
        unsigned char *gttdata = NULL;
        unsigned long gttsize = this->build(&gttdata);

        if (gttdata == NULL) {
            log::error << "Sock Packet: Unable to build packet" << log::endl;
            return false;
        }

        // Send all data
        bool result = sock.sendAll(gttdata, gttsize);
        delete[] gttdata;
        return result;
    }
}
