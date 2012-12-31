#ifndef BLOCKINGQUEUE_H
#define BLOCKINGQUEUE_H

#include <QObject>
#include <QWaitCondition>
#include <QMutex>
#include <deque>

template<typename Data>
class BlockingQueue
{
private:
    QMutex _mutex;
    QWaitCondition _monitor;
    volatile bool _closed;
    std::deque<Data> _queue;

public:
    BlockingQueue()
    {
        _closed = false;
    }

    ~BlockingQueue()
    {
        Close(); // When this is enabled, I get an access violation exception in TryDequeue
    }

    void Close()
    {
        QMutexLocker locker(&_mutex);
        if(!_closed)
        {
            _closed = true;
            _queue.empty();
            _monitor.wakeAll();
        }
    }
    bool empty()
    {
        QMutexLocker locker(&_mutex);
        return _queue.size() == 0;
    }
    int count()
    {
        QMutexLocker locker(&_mutex);
        return _queue.size();
    }

    bool push_back(Data data) { return Enqueue(data); }
    bool Enqueue(Data data)
    {
        QMutexLocker locker(&_mutex);

        // Make sure that the queue is not closed
        if(_closed)
        {
            return false;
        }

        _queue.push_back(data);

        // Signal all the waiting threads
        if(_queue.size()==1)
        {
            _monitor.wakeAll();
        }
        return true;
    }
    bool push_front(Data data) { return EnqueueFront(data); }
    bool EnqueueFront(Data data)
    {
        QMutexLocker locker(&_mutex);

        // Make sure that the queue is not closed
        if(_closed)
        {
            return false;
        }

        _queue.push_front(data);

        // Signal all the waiting threads
        if(_queue.size()==1)
        {
            _monitor.wakeAll();
        }

        return true;
    }

    bool wait(unsigned long time = ULONG_MAX)
    {
        QMutexLocker locker(&_mutex);

        // Block until something goes into the queue
        // or until the queue is closed
        while(_queue.empty())
        {
            if(_closed || !_monitor.wait(&_mutex, time)) // <-- Access violation if I call close in the destructor
            {
                return false;
            }
        }
        return true;
    }

    bool waitOnce(unsigned long time = ULONG_MAX)
    {
        QMutexLocker locker(&_mutex);

        // Block until something goes into the queue
        // or until the queue is closed
        if(_queue.empty())
        {
            if(_closed || !_monitor.wait(&_mutex, time)) // <-- Access violation if I call close in the destructor
            {
                return false;
            }
        }
        return true;
    }

    bool UnsafeDequeue(Data& value, unsigned long time = ULONG_MAX)
    {
        if(!_queue.empty())
        {
            value = _queue.front();
            _queue.pop_front();
            return true;
        }
        return false;
    }

    bool TryDequeue(Data& value, unsigned long time = ULONG_MAX)
    {
        QMutexLocker locker(&_mutex);

        // Block until something goes into the queue
        // or until the queue is closed
        while(_queue.empty())
        {
            if(_closed || !_monitor.wait(&_mutex, time)) // <-- Access violation if I call close in the destructor
            {
                return false;
            }
        }

        // Dequeue the next item from the queue
        value = _queue.front();
        _queue.pop_front();
        return true;
    }
    typename std::deque<Data>::const_iterator begin() { return _queue.begin();}
    typename std::deque<Data>::const_iterator end() { return _queue.end();}
};

#endif // BLOCKINGQUEUE_H
