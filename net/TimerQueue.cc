#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/timerfd.h>
#include "EventLoop.h"
#include "TimerQueue.h"
#include "../base/Logging.h"
#include "../base/Timestamp.h"

int createTimerfd()
{
    int timerfd = timerfd_create(CLOCK_MONOTONIC,TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0)
    {
        LOG_ERROR << "Failed in timerfd_create";
    }
    return timerfd;
}

struct timespec howMuchTimeFromNow(Timestamp when)
{
    int64_t microseconds = when.microSecondsSinceEpoch() - 
        Timestamp::now().microSecondsSinceEpoch();
    if(microseconds < 100)
        microseconds = 100;
    struct timespec tm;
    tm.tv_sec = (time_t)microseconds/Timestamp::kMicroSecondsPerSecond;
    tm.tv_nsec = static_cast<long>((microseconds % Timestamp::kMicroSecondsPerSecond) * 1000);
    return tm;
}

void readTimerfd(int timerfd, Timestamp now)
{
    uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof howmany);
    LOG_TRACE << "TimerQueue::handleRead() " << howmany << " at " << now.toString();
    if (n != sizeof howmany)
    {
        LOG_ERROR << "TimerQueue::handleRead() reads " << n << " bytes instead of 8";
    }
}

void setTimerfd(int timerfd, Timestamp expiration)
{
    struct itimerspec newValue;
    struct itimerspec oldValue;
    bzero(&newValue, sizeof newValue);
    bzero(&oldValue, sizeof oldValue);
    newValue.it_value = howMuchTimeFromNow(expiration);
    int ret = ::timerfd_settime(timerfd, 0, &newValue, &oldValue);
    if (ret)
    {
        LOG_ERROR << "timerfd_settime()";
    }
}

TimerQueue::TimerQueue(EventLoop* loop)
: _loop(loop)
, _timerfd (createTimerfd())
, _timerfdChannel(_loop,_timerfd)
, _timers()
{
    _timerfdChannel.setReadCallback(std::bind(&TimerQueue::handleRead,this));
    _timerfdChannel.enableReading();
}

TimerQueue::~TimerQueue()
{
    close(_timerfd);
    for (TimerList::iterator it = _timers.begin();it != _timers.end(); ++it)
    {
        delete it->second;
    }
}

Timer* TimerQueue::addTimer(const TimerCallback& cb,Timestamp when,double interval)
{
    Timer* timer = new Timer(cb,when,interval);
    _loop->runInLoop(std::bind(&TimerQueue::addTimerInLoop, this, timer));
    return timer;
}

void TimerQueue::addTimerInLoop(Timer* timer)
{
    _loop->assertInLoopThread();
    bool earliestChanged = insert(timer);
    if(earliestChanged)
    {
        setTimerfd(_timerfd,timer->expiration());
    }
}

void TimerQueue::handleRead()
{
    _loop->assertInLoopThread();
    Timestamp now(Timestamp::now());
    readTimerfd(_timerfd, now);
    std::vector<Entry> expired = getExpired(now);
    for(auto it : expired)
    {
        it.second->run();
    }
    reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::getExpired(Timestamp now)
{
    std::vector<Entry> expired;
    Entry sentry = std::make_pair(now, reinterpret_cast<Timer*>(UINTPTR_MAX));
    TimerList::iterator it = _timers.lower_bound(sentry);
    assert(it == _timers.end() || now < it->first);
    std::copy(_timers.begin(), it, back_inserter(expired));
    _timers.erase(_timers.begin(),it);
    return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now)
{
    Timestamp nextExpire;
    for (auto it = expired.begin();it != expired.end(); ++it)
    {
        if(it->second->repeat())
        {
            it->second->restart(now);
            insert(it->second);
        }
        else
        {
            delete it->second;
        }
    }
    if(!_timers.empty())
    {
        nextExpire = _timers.begin()->second->expiration();
    }
    if (nextExpire.valid())
    {
        setTimerfd(_timerfd, nextExpire);
    }
}

bool TimerQueue::insert(Timer* timer)
{
    bool earliestChanged = false;
    Timestamp when = timer->expiration();
    TimerList::iterator it = _timers.begin();
    if (it == _timers.end() || when < it->first)
    {
        earliestChanged = true;
    }
    std::pair<TimerList::iterator, bool> result =
                   _timers.insert(std::make_pair(when, timer));
    assert(result.second);
    return earliestChanged;
}



