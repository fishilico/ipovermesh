/**
 * @file blocking-queue-detail.h
 * @brief a blocking queue implementation
 *
 * You should never include this file directly
 */
#ifndef IOM_BLOCKING_QUEUE_DETAIL_H
#define IOM_BLOCKING_QUEUE_DETAIL_H

namespace iom
{

    template<typename T> BlockingQueue<T>::BlockingQueue()
    : opened(true) {
    }

    template<typename T> BlockingQueue<T>::~BlockingQueue() {
        // FIXME: Who frees the elements ?
    }

    template<typename T> void BlockingQueue<T>::close() {
        opened = false;
        cond.notify_all();
    }

    template<typename T> bool BlockingQueue<T>::isOpened() {
        return opened;
    }

    template<typename T> bool BlockingQueue<T>::push(TPtr e) {
        if (!opened) {
            return false;
        }
        {
            boost::lock_guard<boost::mutex> lock(mut);
            fifo.push_back(e);
        }
        cond.notify_one();
        return true;
    }

    template<typename T> bool BlockingQueue<T>::tryPush(TPtr e) {
        if (opened && mut.try_lock()) {
            fifo.push_back(e);
            mut.unlock();
            cond.notify_one();
            return true;
        }
        return false;
    }

    template<typename T> typename BlockingQueue<T>::TPtr BlockingQueue<T>::pop() {
        boost::unique_lock<boost::mutex> lock(mut);
        while (opened && fifo.empty()) {
            cond.wait(lock);
        }
        if (fifo.empty()) {
            return NULL;
        }

        // Continue to pop even if queue is closed
        TPtr result = fifo.front();
        fifo.pop_front();
        return result;
    }

    template<typename T> template<typename time_duration>
    typename BlockingQueue<T>::TPtr BlockingQueue<T>::pop(time_duration duration) {
        boost::unique_lock<boost::mutex> lock(mut);
        if (opened && fifo.empty()) {
            if (!cond.timed_wait(lock, duration))
                return NULL;
        }
        if (fifo.empty()) {
            return NULL;
        }

        // Continue to pop even if queue is closed
        TPtr result = fifo.front();
        fifo.pop_front();
        return result;
    }

    template<typename T> typename BlockingQueue<T>::TPtr BlockingQueue<T>::tryPop() {
        TPtr result = NULL;
        if (mut.try_lock()) {
            if (!fifo.empty()) {
                result = fifo.front();
                fifo.pop_front();
            }
            mut.unlock();
        }
        return result;
    }

    template<typename T> typename BlockingQueue<T>::TQueuePtr BlockingQueue<T>::flush() {
        TQueuePtr result;
        boost::unique_lock<boost::mutex> lock(mut);
        while (opened && fifo.empty()) {
            cond.wait(lock);
        }
        if (!fifo.empty()) {
            result = TQueuePtr(new TQueue(fifo));
            fifo.clear();
        }
        return result;
    }

    template<typename T> template<typename time_duration>
    typename BlockingQueue<T>::TQueue* BlockingQueue<T>::flush(time_duration duration) {
        TQueue *result = NULL;
        boost::unique_lock<boost::mutex> lock(mut);
        while (opened && fifo.empty()) {
            if (!cond.timed_wait(lock, duration))
                return result;
        }
        if (!fifo.empty()) {
            result = new TQueue(fifo);
            fifo.clear();
        }
        return result;
    }

    template<typename T> typename BlockingQueue<T>::TQueue* BlockingQueue<T>::tryFlush() {
        TQueue *result = NULL;
        if (mut.try_lock()) {
            if (!fifo.empty()) {
                result = new TQueue(fifo);
                fifo.clear();
            }
            mut.unlock();
        }
        return result;
    }

    template<typename T> size_t BlockingQueue<T>::size() {
        boost::unique_lock<boost::mutex> lock(mut);
        return fifo.size();
    }

    template<typename T> bool BlockingQueue<T>::empty() {
        boost::unique_lock<boost::mutex> lock(mut);
        return fifo.empty();
    }
}

#endif /* IOM_BLOCKING_QUEUE_DETAIL_H */
