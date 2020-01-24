#pragma once
#include <deque>
#include <assert.h>
#include "Mutex.h"
#include "Condition.h"

//有容量有锁的任务队列，put时超过容量直接把任务抛弃，返回-1，让调用线程记日志。

template <typename T>
class BlockingQueue : noncopyable
{
public:
    BlockingQueue(int capacity)
    : _capacity(capacity)
    , _mutex()
    , _cond(_mutex)
    {}
    
    int put(const T& task)
    {
        MutexLock lock(_mutex);
        if(_queue.size() >= _capacity)
            return -1;
        _queue.push_back(task);
        _cond.notify();
        return 0;
    }

    T take()
    {
        MutexLock lock(_mutex);
        while(_queue.empty())
        {
            _cond.wait();
        }
        assert(!_queue.empty());
        T front(std::move(_queue.front()));
        _queue.pop_front();
        return front;
    }
    
     size_t size() const
     {
        MutexLockGuard lock(_mutex);
        return _queue.size();
     }

     void notifyAll() {  _cond.notifyAll();  }

private:
    int _capacity;
    MutexLock _mutex;
    Condition _cond;
    std::deque<T> _queue;
};

