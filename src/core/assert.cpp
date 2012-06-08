/**
 * This file defines a mandatory function to use BOOST_ASSERT
 */
#include <boost/assert.hpp>
#include "log.h"

namespace boost
{

    void assertion_failed(const char *expr, const char *function, const char* file, long line) {
        iom::log::fatal << "Assertion failed: " << expr << "\n"
            << "in " << function << " at " << file << ":" << line << iom::log::endl;
    }
}
