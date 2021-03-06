/**
 * @file gttparser.h
 * @brief Generic Text Template implementation
 */

#ifndef IOM_PARSER_GTTPARSER_H
#define IOM_PARSER_GTTPARSER_H

#include "gttpacket.h"
#include "lineparser.h"
#include <ctype.h>
#include <string>

namespace iom
{
    /**
     * @class GTTParser
     *
     * @brief GTT implementation
     */
    class GTTParser
    {
    public:
        /**
         * @brief Constructor
         */
        GTTParser();
        /**
         * @brief Destructor
         */
        ~GTTParser();

        /**
         * @brief reset internal state
         */
        void reset();

        /**
         * @brief get error, if any
         * @return true if there is an error
         */
        bool getError(std::string& userErr) const;

        /**
         * @brief eat some fresh data
         * @param data data to eat
         * @param size available size
         */
        void eat(const unsigned char *data, long size);

        /**
         * @brief get the last read packet
         * @return a packet or NULL. If not NULL, the caller is responsible of delete this packet
         */
        GTTPacket* getPacket();

    protected:
        /**
         * @brief Current GTT Packet
         *
         * This is protected to allow children parsers to fill this structure.
         */
        GTTPacket *currentPkt;

        /**
         * @brief Parse the first line of GTT
         * @param line first line
         * @throw ParserException on errors
         */
        virtual void parseFirstLine(const std::string& line);

        /**
         * @brief Parse a header line of GTT
         * @param line header line
         * @throw ParserException on errors
         */
        virtual void parseHeaderLine(const std::string& line);

    private:
        std::string errorMessage;
        bool hasError;

        // Use LineParser to parse lines
        LineParser lineParser;

        // Position in data
        enum DataPosition
        {
            dataposFirstLine,
            dataposHeaders,
            dataposContent
        } datapos;

        /**
         * @brief start a new packet, without cleaning read data
         */
        void startPacket();

        void setError(const char *msg);
    };
}

#endif /* IOM_PARSER_GTTPARSER_H */

