/**
 * @file exception.h
 * @brief Exceptions management system
 */
#ifndef IOM_EXCEPTION_H
#define IOM_EXCEPTION_H

#include <string>

namespace iom
{
    /**
     * @class Exception
     */
    class Exception
    {
    public:
        /**
         * @brief Build an exception
         * @param type
         * @param module
         * @param message
         */
        Exception(const char *type, const char *module, const char *message);

        /**
         * @brief Print an exception
         * @param os output stream
         * @param e
         */
        friend std::ostream& operator<<(std::ostream& os, const Exception& e);

        /**
         * @brief Get message
         */
        const std::string& getMessage() const;

    protected:
        // Exception type
        std::string type;

        // Module which throws the exception
        std::string module;

        // @brief Exception text
        std::string message;
    };
    class FailException : public Exception
    {
    public:
        FailException(const char *module, const char *message);
    };
    /**
     * @class ErrException
     * @brief Throw a FailException with the message from errno
     */
    class ErrException : public FailException
    {
    public:
        ErrException(const char *module, const char *fct);
    };
    class MinorException : public Exception
    {
    public:
        MinorException(const char *module, const char *message);
    };
    class ParserException : public Exception
    {
    public:
        ParserException(const char *module, const char *message);
    };
}

#endif /* IOM_EXCEPTION_H */
