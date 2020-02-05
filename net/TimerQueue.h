#pragma once
#include <set>
#include <vector>
#include "Channel.h"
#include "Timer.h"
#include "../base/noncopyable.h"
#include "../base/Mutex.h"

class EventLoop;
class Timer;

class TimerQueue
{
public:
    typedef std::function<void()> TimerCallback;
    TimerQueue(EventLoop* loop);
    ~TimerQueue();

    Timer* addTimer(const TimerCallback& cb,Timestamp when,double interval);

private:
    typedef std::pair<Timestamp, Timer*> Entry;
    typedef std::set<Entry> TimerList;

    void addTimerInLoop(Timer* timer);

    void handleRead();

    std::vector<Entry> getExpired(Timestamp now);

    void reset(const std::vector<Entry>& expired, Timestamp now);

    bool insert(Timer* timer);

    EventLoop* _loop;
    const int _timerfd;
    Channel _timerfdChannel;
    TimerList _timers;
};

