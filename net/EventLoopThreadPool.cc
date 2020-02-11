#include "EventLoopThreadPool.h"
#include "EventLoop.h"
#include "EventLoopThread.h"
#include <assert.h>

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop)
: _baseLoop(baseLoop)
, _started(false)
, _numThreads(0)
, _next(0)
{}

EventLoopThreadPool::~EventLoopThreadPool()
{

}

void EventLoopThreadPool::start()
{
    assert(!_started);
    _baseLoop->assertInLoopThread();
    _started = true;
    for (int i = 0; i < _numThreads; ++i)
    {
        EventLoopThread* t = new EventLoopThread;
        _threads.push_back(std::unique_ptr<EventLoopThread>(t));
        _loops.push_back(t->startLoop());
    }
}

EventLoop* EventLoopThreadPool::getNextLoop()
{
    _baseLoop->assertInLoopThread();
    EventLoop* loop = _baseLoop;
    if (!_loops.empty())
    {
        loop = _loops[_next];
        ++_next;
        if((size_t)_next >= _loops.size())
        {
            _next = 0;
        }
    }
    return loop;
}
