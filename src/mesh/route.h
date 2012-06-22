#ifndef ROUTE_H
#define ROUTE_H

#include "../net/address.h"
#include <boost/date_time/posix_time/posix_time.hpp>

namespace iom {

    class Route
    {
    public:
        Route(const Address &destination, const Address &nextHop, int hops);
        bool hasExpired();

        const Address destination;
        const Address nextHop;
        const int hops;

    private:
        const boost::posix_time::ptime createTime;
    };
}

#endif // ROUTE_H
