#include "EventLoopThread.h"
#include "EventLoop.h"



EventLoopThread::EventLoopThread()
: _exiting(false)
, _thread(std::bind(&EventLoopThread::threadFunc,this))
, _loop(NULL)
, _cond(_mutex)
{}

EventLoopThread::~EventLoopThread()
{
    _exiting = true;
    _loop->quit();
    _thread.join();
}

EventLoop* EventLoopThread::startLoop()
{
    _thread.start();
    {
        MutexLockGuard lock(_mutex);
        while(_loop == NULL)
        {
            _cond.wait();
        }
    }
    return _loop;
}

void EventLoopThread::threadFunc()
{
    EventLoop loop;
    {
        MutexLockGuard lock(_mutex);
        _loop = &loop;
        _cond.notify();
    }
    loop.loop();
}
