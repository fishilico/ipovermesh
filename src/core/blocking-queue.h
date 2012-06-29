/**
 * @file blocking-queue.h
 * @brief a blocking queue definition.
 */
#ifndef IOM_BLOCKING_QUEUE_H
#define IOM_BLOCKING_QUEUE_H

#include <boost/date_time/time_duration.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <queue>

/**
 * @class BlockingQueue
 *
 * @brief A thread-safe fifo container for pointers
 *
 * Access to the blocking queue can be synchronous or asynchronous which makes it good
 * for passing messages from thread to threads. It stores pointers to the template parameter.
 * The thread popping a pointer is responsible for deleting it however when the queue is
 * destroyed it will delete all the objects it still has in memory. Thus the pushing thread
 * should not keep a reference to an object "stored" in the queue.
 *
 * @tparam T the base type of the pointers contained in the BlockingQueue
 */

namespace iom
{
    template<typename T> class BlockingQueue : private boost::noncopyable
    {
    public:
        typedef boost::shared_ptr<T> TPtr;
        typedef std::deque<TPtr> TQueue;
        typedef boost::shared_ptr<TQueue> TQueuePtr;

        /**
         * @brief Constructor
         */
        BlockingQueue();

        /**
         * @brief Destructor
         *
         * It will automatically delete all the remaining elements stored in the queue.
         */
        ~BlockingQueue();

        /**
         * @brief Close the queue prior to deletion
         *
         * This is to be used to notify the threads waiting on the queue
         * as it makes all the pending pop() return NULL.
         */
        void close();

        /**
         * @brief Return false if the queue was closed
         * @return returns true if close() has been called, false otherwise
         */
        bool isOpened();

        /**
         * @brief Push an element on the queue, the synchronous way
         * @param e a pointer to the element to push
         * @return false if the queue is closed, true otherwise
         */
        bool push(T *e);

        /**
         * @brief Push an element on the queue, the synchronous way
         * @param e a pointer to the element to push
         * @return false if the queue is closed, true otherwise
         */
        bool push(TPtr e);

        /**
         * @brief Push an element on the queue, the best-effort way
         *
         * This is not a really asynchronous call, it will try to lock the queue
         * and then push the element.
         *
         * @param e a pointer to the element to push
         * @return true if the queue is not closed and the element was pushed, false otherwise
         */
        bool tryPush(TPtr e);

        /**
         * @brief Pop an element, the synchronous way
         * @return An element or NULL if the queue gets closed
         */
        TPtr pop();

        /**
         * @brief Pop an element, the timed way
         *
         * This version of pop waits for an element or the specified time before returning.
         *
         * @param duration time to wait
         * @return An element or NULL if the queue gets closed or the time runs out
         */
        template<typename time_duration> TPtr pop(time_duration duration);

        /**
         * @brief Pop an element, the best-effort way
         *
         * This is not a really asynchronous call, it will try to lock the queue
         * and then pop an element.
         *
         * @return An element or NULL if the queue was closed or if it could not get the lock
         */
        TPtr tryPop();

        /**
         * @brief Return a copy of the queue and empty it, the synchronous way
         * @return A copy of the content of the queue or NULL if the queue gets closed
         */
        TQueuePtr flush();

        /**
         * @brief Return a copy of the queue and empty it, the timed way
         *
         * This version of flush waits for the lock or the specified time before returning.
         *
         * @param duration time to wait in milliseconds
         * @return A copy of the content of the queue or NULL if the queue gets closed or the time runs out
         */
        template<typename time_duration> TQueue* flush(time_duration duration);

        /**
         * @brief Returns a copy of the queue and empty it, the best-effort way
         *
         * This is not a really asynchronous call, it will try to lock the queue
         * and then flush it.
         *
         * @return A copy of the content of the queue or NULL if the queue was closed or if it could not get the lock
         */
        TQueue* tryFlush();

        /**
         * @brief Get queue size
         * @return size
         */
        size_t size();

        /**
         * @brief Tell wether it is empty
         * @return true if it is empty
         */
        bool empty();

    private:
        boost::condition_variable cond;
        boost::mutex mut;
        TQueue fifo;
        bool opened;

    };
}

#include "blocking-queue-detail.h"

#endif /* IOM_BLOCKING_QUEUE_H */
