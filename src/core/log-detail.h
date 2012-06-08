/**
 * @file log-detail.h
 * @brief Template implementation of log.h
 */

#ifndef IOM_LOG_DETAIL_H
#define IOM_LOG_DETAIL_H

#include <boost/assert.hpp>

namespace iom
{

    namespace log
    {

        template<typename T> Stream& Stream::operator<<(const T& arg) {
            std::ostringstream str;
            str << arg;
            return this->appendMessage(str.str());
        }
    }
}

#endif /* IOM_LOG_DETAIL_H */

