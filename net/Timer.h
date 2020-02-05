#pragma once
#include "../base/noncopyable.h"
#include "../base/Timestamp.h"
#include <functional>

class Timer
{
public:
    typedef std::function<void()> TimerCallback;
    Timer(const TimerCallback& cb, Timestamp when, double interval)
    : _callback(cb)
    , _expiration(when)
    , _interval(interval)
    , _repeat(interval > 0.0)
    {}
    
    void run() const {  _callback(); }

    Timestamp expiration() const {  return _expiration; }

    bool repeat() const {  return _repeat; }

    void restart(Timestamp now)
    {
        if(_repeat)
            _expiration = addTime(now,_interval);
        else
            _expiration = Timestamp::invalid();
    }

private:
    const TimerCallback _callback;
    Timestamp _expiration;
    const double _interval;
    const bool _repeat;
};

