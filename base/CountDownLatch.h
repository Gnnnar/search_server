#pragma once
#include "Mutex.h"
#include "Condition.h"

class CountDownLatch
{
public:
    CountDownLatch(int count)
    : _mutex()
    , _cond(_mutex)
    , _count(count)
    {}
    ~CountDownLatch() = default;

    void wait();

    void cutDown();

private:
    MutexLock _mutex;
    Condition _cond;
    int _count;
};

