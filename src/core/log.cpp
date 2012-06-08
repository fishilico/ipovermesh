#include "log.h"
#include <errno.h>
#include <cstring>

namespace iom
{
    namespace log
    {
        const std::string logLevelNames[5] = {
            "FATAL",
            "ERROR",
            "WARN",
            "INFO",
            "DEBUG"
        };

        Stream fatal(LOGLVL_FATAL);
        Stream error(LOGLVL_ERROR);
        Stream warn(LOGLVL_WARN);
        Stream info(LOGLVL_INFO);
        Stream debug(LOGLVL_DEBUG);

        EndlStruct endl;
        ErrstdStruct errstd;

        boost::thread_specific_ptr<ThreadInfo> threadInfos;

        Worker worker;

        /**
         * @brief allocate and return a thread info structure
         * @return 
         */
        static ThreadInfo* getThreadInfo() {
            ThreadInfo* ti = threadInfos.get();
            if (ti == NULL) {
                ti = new ThreadInfo;
                threadInfos.reset(ti);
            }
            return ti;
        }

        ThreadInfo::ThreadInfo()
        :name("Unnamed thread") {
        }

        void init(int flags, const std::string& file) {
            worker.init(flags, file);
        }

        void flush() {
            worker.flush(false);
        }

        void waitFlush() {
            worker.flush(true);
        }

        void flushAndQuit() {
            worker.quit();
        }

        void setThreadName(const std::string& name, int id) {
            if (id >= 0) {
                std::ostringstream str;
                str << name << " " << id;
                getThreadInfo()->name = str.str();
            } else {
                getThreadInfo()->name = name;
            }
        }

        Stream::Stream(Level level) :level(level) {
        }

        Stream::~Stream() {
        }

        Stream& Stream::operator<<(const std::string& str) {
            return this->appendMessage(str);
        }

        Stream& Stream::operator<<(const char str[]) {
            return this->appendMessage(str);
        }

        Stream& Stream::operator<<(const EndlStruct& f) {
            if (!worker.initialized) {
                // No log system : fallback
                std::cerr << "[log fallback] " << std::endl;
                return *this;
            }
            BOOST_VERIFY(level < loglvl_count);
            const std::string& message = getThreadInfo()->logstreams[level].str();
            worker.write(message, level);
            getThreadInfo()->logstreams[level].str("");
            return *this;
        }

        Stream& Stream::operator<<(const ErrstdStruct& f) {
            int err_code = errno;
            static boost::mutex strerrorMutex;
            boost::lock_guard<boost::mutex> lock(strerrorMutex);
            const char *err_str = std::strerror(err_code);
            return *this << err_str << " (errno #" << err_code << ")";
        }

        Stream& Stream::appendMessage(const std::string& str) {
            if (!worker.initialized) {
                // No log system : fallback
                std::cerr << "[log fallback] " << str << std::endl;
                return *this;
            }
            getThreadInfo()->logstreams[level] << str;
            return *this;
        }

        Worker::Worker() :initialized(false), thread(NULL), flags(0) {
        }

        void Worker::init(int flags, const std::string& file) {
            BOOST_VERIFY(!this->initialized);
            this->flags = flags;
            if (!file.empty() && (flags & LOGFILE)) {
                //TODO: Close it
                this->logFile.open(file.c_str());
                BOOST_VERIFY(this->logFile.is_open());
            }
            // start thread
            this->thread = new boost::thread(boost::bind(&Worker::run, this));
            setThreadName("Main");
            this->initialized = true;
        }

        void Worker::write(const std::string& message, Level level) {
            boost::shared_ptr<LogEntry> entry(new LogEntry(level, message));
            entries.push(entry);
        }

        void Worker::flush(bool wait) {
            boost::shared_ptr<LogEntry> entry(new LogEntry(LOGLVL_FLUSH));

            if (wait)
                entry->isTraited = boost::shared_ptr<SharedBool > (new SharedBool(false));

            entries.push(entry);

            //The entry will get destroyed but not the condition
            if (entry->isTraited.get() == NULL)
                entry->isTraited->wait(true);
        }

        void Worker::quit() {
            entries.close();
            if (thread) {
                thread->join();
                delete thread;
                thread = NULL;
            }
        }

        void Worker::run() {
            setThreadName("Log");
            info << "Opening log" << endl;
            while (entries.isOpened() || !entries.empty()) {
                //Take all the elements from the queue with only one call
                BlockingQueue<LogEntry>::TQueuePtr entriesQueue = entries.flush();
                if (entriesQueue == NULL) continue;

                while (!entriesQueue->empty()) {
                    boost::shared_ptr<LogEntry> entry = entriesQueue->front();
                    entriesQueue->pop_front();
                    if (entry->level == LOGLVL_FLUSH) {
                        this->internalFlush();
                    } else if (entry->level < loglvl_count) {
                        this->internalPrint(*entry);
                    }
                    if (entry->isTraited.get() != NULL) {
                        entry->isTraited->set(true);
                    }
                }
            }
            // quit
            this->internalPrint(LogEntry(LOGLVL_INFO, "Closing log"));
            this->internalFlush();
        }

        void Worker::internalFlush() {
            if (flags & CONSOLE) std::cout << std::flush;
            if (flags & ERRORCONSOLE) std::cerr << std::flush;
            if (flags & LOGFILE) logFile << std::flush;
        }

        void Worker::internalPrint(const LogEntry& entry) {
            char time_buffer[20];
            char date_buffer[20];

            //This returns a pointer to a static struct, I hope I'm the only one using it
            //TODO: fix this!
            tm* timeinfo = localtime(&entry.time);
            strftime(time_buffer, 20, "%H:%M:%S", timeinfo);
            strftime(date_buffer, 20, "%Y-%m-%d", timeinfo);

            //Make the part with the thread's name
            std::ostringstream info_buffer;
            info_buffer << "[" << logLevelNames[entry.level] << "] ";
            info_buffer << "[" << entry.thread_name << "]";

            //TODO: Flush control commands
            //Do the actual IO
            if (this->flags & CONSOLE) {
                std::cout << "[" << time_buffer << "] " << info_buffer.str()
                    << " " << entry.str << std::endl;
            }
            if (this->flags & ERRORCONSOLE) {
                std::cerr << "[" << time_buffer << "] " << info_buffer.str()
                    << " " << entry.str << std::endl;
            }
            if (this->flags & LOGFILE) {
                logFile << "[" << date_buffer << " " << time_buffer << "] " << info_buffer.str()
                    << " " << entry.str << std::endl;
            }
        }

        Worker::LogEntry::LogEntry(Level level, const std::string& str)
        :str(str), level(level) {
            time = ::time(NULL);
            thread_name = getThreadInfo()->name;
        }
    }
}
