#pragma once
#include "../base/Mutex.h"
#include "../base/Condition.h"
#include "../base/Thread.h"


class EventLoop;

class EventLoopThread : noncopyable
{
public:
    EventLoopThread();
    ~EventLoopThread();

    EventLoop* startLoop();

private:
    void threadFunc();
    bool _exiting;
    Thread _thread;
    EventLoop* _loop;
    MutexLock _mutex;
    Condition _cond;
};

