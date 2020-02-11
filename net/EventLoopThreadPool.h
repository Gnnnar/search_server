#pragma once
#include <vector>
#include <functional>
#include <memory>
#include "../base/Mutex.h"
#include "../base/Condition.h"
#include "../base/Thread.h"
#include "../base/noncopyable.h"

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : noncopyable
{
public:
    EventLoopThreadPool(EventLoop* baseLoop);
    ~EventLoopThreadPool();

    void setThreadNum(int numThreads) { _numThreads = numThreads;  }

    void start();

    EventLoop* getNextLoop();

private:
    EventLoop* _baseLoop;
    bool _started;
    int _numThreads;
    int _next;
    std::vector<std::unique_ptr<EventLoopThread>> _threads;
    std::vector<EventLoop*> _loops;
};

