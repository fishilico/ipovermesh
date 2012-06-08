#include "exception.h"
#include "log.h"
#include <errno.h>
#include <iostream>
#include <sstream>
#include <string.h>

namespace iom
{

    Exception::Exception(const char *type, const char *module,
        const char *message)
    :type(type), module(module), message(message) {
    }

    std::ostream& operator<<(std::ostream& os, const Exception& e) {
        return os << e.type << " in module " << e.module << ": " << e.message << "\n";
    }

    const std::string& Exception::getMessage() const {
        return message;
    }

    FailException::FailException(const char *module, const char *message)
    :Exception("IOM fails", module, message) {
    }

    ErrException::ErrException(const char *module, const char *fct)
    :FailException(module, "") {
        std::stringstream msg;
        log::error << fct << ": " << log::errstd << log::endl;
        msg << "Error with " << fct << std::endl;
        message.append(msg.str().c_str());
    }

    MinorException::MinorException(const char *module, const char* message)
    :Exception("Internal minor error", module, message) {
    }

    ParserException::ParserException(const char *module, const char* message)
    :Exception("Parser error", module, message) {
    }
}
