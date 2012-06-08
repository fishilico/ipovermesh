/**
 * @file sharedbool.h
 * @brief Shared boolean abstraction layer
 */

#ifndef IOM_SHAREDBOOL_H
#define IOM_SHAREDBOOL_H

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>

namespace iom
{
    /**
     * @class SharedBool
     * @brief Shared boolean class
     */
    class SharedBool : private boost::noncopyable
    {
    public:
        SharedBool(bool val = false);

        /**
         * @brief Set to this value
         * @param newval new value
         */
        void set(bool newval);

        /**
         * @brief Wait to be set to that value
         * @param waited value to be waited
         * @return true when val = waited
         */
        bool wait(bool waited);

    private:
        bool val;
        boost::mutex mut;
        boost::condition_variable cond;
    };
}

#endif /* IOM_SHAREDBOOL_H */

