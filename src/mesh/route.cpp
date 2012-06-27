#include "route.h"
#include <time.h>

#define EXPIRATION_DELAY boost::posix_time::seconds(60)

namespace iom {

    Route::Route(const Address &destination, const Address &nextHop, unsigned int hops) :
        destination(destination), nextHop(nextHop), hops(hops), createTime(boost::posix_time::second_clock::local_time())
    {
    }

    bool Route::hasExpired()
    {
        return boost::posix_time::second_clock::local_time() - createTime > EXPIRATION_DELAY;
    }
}
