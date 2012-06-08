/**
 * @file log.h
 * @brief A thread safe logging system.
 */
#ifndef IOM_LOG_H
#define IOM_LOG_H

#include <boost/thread.hpp>
#include <boost/thread/tss.hpp>
#include <fstream>
#include <sstream>
#include <vector>
#include "blocking-queue.h"
#include "sharedbool.h"

namespace iom
{
    namespace log
    {
        // Define a Stream class to handle << operators nicely
        // This Struct is the endLog struct that ends the log line
        // (a log line may be multiple lines)
        struct EndlStruct
        {
        };
        struct ErrstdStruct
        {
        };

        /**
         * @brief An agument to be passed to a log stream to finish a log entry
         */
        extern EndlStruct endl;

        /**
         * @brief An agument to be passed to a log stream to print the standard error code nicely
         */
        extern ErrstdStruct errstd;

        //Debug levels
        enum Level
        {
            LOGLVL_FATAL = 0,
            LOGLVL_ERROR,
            LOGLVL_WARN,
            LOGLVL_INFO,
            LOGLVL_DEBUG,
            // Other internal levels
            LOGLVL_FLUSH
        };

        const unsigned int loglvl_count = 5;
        /**
         * @brief Per-thread structure
         */
        struct ThreadInfo
        {
            ThreadInfo();
            std::string name;
            std::ostringstream logstreams[loglvl_count];
        };
        extern boost::thread_specific_ptr<ThreadInfo> threadInfos;

        // Logger flags : Where is the output written ?
        enum
        {
            // to std::cout
            CONSOLE = 1,
            // to std::cerr
            ERRORCONSOLE = 2,
            // to a file
            LOGFILE = 4
        };

        /**
         * @brief initializes the logger
         * @param flags the initialisation flags
         * @param file (optional) the file to log to
         */
        void init(int flags = CONSOLE, const std::string& file = "");

        /**
         * @brief Ask the logging system to flush the logs to the logfile and the log streams -- Non-blocking variant
         *
         * The flags are CONSOLE, ERRORCONSOLE and LOGFILE
         */
        void flush();

        /**
         * @brief Ask the logging system to flush the logs to the logfile and the log streams -- Blocking variant
         */
        void waitFlush();

        /**
         * @brief Quit the logger nicely
         */
        void flushAndQuit();

        /**
         * @brief Set thread name
         * @param name
         * @param id a optional positive number
         */
        void setThreadName(const std::string& name, int id = -1);
        /**
         * @class Stream
         * @brief The definition of a logging stream to have a nice interface
         */
        class Stream : private boost::noncopyable
        {
        public:
            /**
             * @brief Constructor
             * @param level stream logging level
             */
            Stream(Level level);

            /**
             * @brief Destructor
             */
            ~Stream();

            /**
             * @brief overloads << to accept almost anything
             */
            template<typename T> Stream& operator<<(const T& arg);
            /**
             * @brief Avoid to define a template for std::string
             * @param str message
             * @return this
             */
            Stream& operator<<(const std::string& str);
            /**
             * @brief Avoid to define a template for each char[N] types
             * @param str message
             * @return this
             */
            Stream& operator<<(const char str[]);
            /**
             * @brief Message terminator
             * @param f EndlStruct
             * @return this
             */
            Stream& operator<<(const EndlStruct& f);
            /**
             * @brief Add standard error to log message
             * @param f ErrstdStruct
             * @return this
             */
            Stream& operator<<(const ErrstdStruct& f);
            
            /**
             * @brief Append a message
             * @param str message
             * @return this
             */
            Stream& appendMessage(const std::string& str);

        private:
            // Log level
            Level level;
        };

        // Here the declaration of the different log streams
        extern Stream debug;
        extern Stream info;
        extern Stream warn;
        extern Stream error;
        extern Stream fatal;
        /**
         * @class Worker
         * @brief Main interface to write log message with an unique log worker
         */
        class Worker : private boost::noncopyable
        {
        public:
            Worker();
            void init(int flags = CONSOLE, const std::string& file = "");
            void write(const std::string& message, Level level);
            void flush(bool wait);
            void quit();
            
            friend Stream& Stream::operator<<(const EndlStruct& f);
            friend Stream& Stream::appendMessage(const std::string& msg);

        private:
            struct LogEntry
            {
                std::string str;
                Level level;
                time_t time;
                std::string thread_name;
                boost::shared_ptr<SharedBool> isTraited;
                LogEntry(Level level, const std::string& str = "");
            };

            // Tell wether log is initialized or not
            bool initialized;

            void run();
            boost::thread *thread;

            BlockingQueue<LogEntry> entries;
            int flags;
            std::ofstream logFile;

            void internalFlush();

            void internalPrint(const LogEntry& entry);
        };

        extern Worker worker;
    }    
}

#include "log-detail.h"

#endif /* IOM_LOG_H */
