#include <iostream>
#include <assert.h>
#include <poll.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include <sys/eventfd.h>
#include "../base/Logging.h"
#include "EventLoop.h"
#include "EPoller.h"
#include "Channel.h"

__thread EventLoop* t_loopInThisThread = 0;
const int kPollTimeMs = 10000;
static int createEventfd()
{
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0)
    {
        LOG_ERROR << "Failed in eventfd";
        abort();
    }
    return evtfd;
}


EventLoop::EventLoop()
    : _looping(false)
    , _quit(false)
    , _callingPendingFunctors(false)
    , _thead_id(CurrentThread::tid())
    , _poller(new EPoller(this))
    , _timerQueue(new TimerQueue(this))
    , _wakeupFd(createEventfd())
      , _wakeupChannel(new Channel(this,_wakeupFd))
{
    LOG_TRACE << "EventLoop created " << (EventLoop*)this << " in thread " << _thead_id;
    if(t_loopInThisThread != 0)
    {
        LOG_FATAL << "Another EventLoop " << t_loopInThisThread
            << " exists in this thread " << _thead_id;
    }else
        t_loopInThisThread = this;
    _wakeupChannel->setReadCallback(std::bind(&EventLoop::handleRead,this));
    _wakeupChannel->enableReading();
}

EventLoop::~EventLoop()
{
    assert(_looping == false);
    close(_wakeupFd);
    t_loopInThisThread = NULL;
}

void EventLoop::loop()
{
    assert(_looping == false);
    assertInLoopThread();
    _looping = true;
    _quit = false;
    while(!_quit)
    {
        _activeChannels.clear();
        _pollReturnTime = _poller->poll(kPollTimeMs,&_activeChannels);
        for(auto & it : _activeChannels)
        {
            it->handleEvent(_pollReturnTime);
        }
        doPendingFunctors();
    }
    LOG_TRACE << "EventLoop " << (EventLoop*)this << " stop looping";
    _looping = false;
}

void EventLoop::quit()
{
    _quit = true;
    if (!isInLoopThread())
    {
        wakeup();
    }
}

void EventLoop::runInLoop(const Functor &cb)
{
    if (isInLoopThread())
    {
        cb();
    }
    else
    {
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(const Functor& cb)
{
    {
        MutexLockGuard lock(_mutex);
        _pendingFunctors.push_back(cb);
    }
    if (!isInLoopThread() || _callingPendingFunctors)
    {
        wakeup();
    }
}

void EventLoop::runAt(const Timestamp& time, const TimerCallback& cb)
{
    _timerQueue->addTimer(cb,time,0.0);
}

void EventLoop::runAfter(double delay, const TimerCallback& cb)
{
    Timestamp tmp(addTime(Timestamp::now(),delay));
    _timerQueue->addTimer(cb,tmp,0.0);
}

void EventLoop::runEvery(double interval, const TimerCallback& cb)
{
    Timestamp tmp(addTime(Timestamp::now(),interval));
    _timerQueue->addTimer(cb,tmp,interval);
}

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = ::write(_wakeupFd, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}

void EventLoop::updateChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    _poller->updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel)
{
    assert(channel->ownerLoop() == this);
    assertInLoopThread();
    _poller->removeChannel(channel);
}


void EventLoop::abortNotInLoopThread()
{
    LOG_FATAL << "EventLoop::abortNotInLoopThread - EventLoop " << (EventLoop*)this
        << " was created in threadId_ = " << _thead_id
        << ", current thread id = " <<  CurrentThread::tid();
}

void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = ::read(_wakeupFd, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG_ERROR << "EventLoop::handleRead() writes " << n << " bytes instead of 8";
    }
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    _callingPendingFunctors = true;
    {
        MutexLockGuard lock(_mutex);
        functors.swap(_pendingFunctors);
    }
    for(auto & cb : functors)
    {
        cb();
    }
    _callingPendingFunctors = false;
}



#if 0
void threadFunc()
{
    printf("threadFunc(): tid = %d\n", CurrentThread::tid());
    EventLoop loop;
    loop.loop();
}

int main()
{
    printf("main(): tid = %d\n", CurrentThread::tid());
    EventLoop loop;
    Thread td(threadFunc);
    td.start();
    loop.loop();
    td.join();
    LOG_FATAL << 1;
    return 0;
}

EventLoop* g_loop;
void threadFunc()
{
    g_loop->loop();

}

int main()
{
    EventLoop loop;
    g_loop = &loop;
    Thread t(threadFunc);
    t.start();
    t.join();
}


EventLoop* g_loop;
void timeout()
{
    printf("Timeout!\n");
    g_loop->quit();
}

int main()
{
    EventLoop loop;
    g_loop = &loop;

    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    Channel channel(&loop, timerfd);
    channel.setReadCallback(timeout);
    channel.enableReading();

    struct itimerspec howlong;
    bzero(&howlong, sizeof howlong);
    howlong.it_value.tv_sec = 5;
    ::timerfd_settime(timerfd, 0, &howlong, NULL);

    loop.loop();

    close(timerfd);
}


#endif

