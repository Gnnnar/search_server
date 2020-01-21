#include "CountDownLatch.h"

void CountDownLatch::wait()
{
    MutexLockGuard lock(_mutex);
    while(_count > 0)
    {
        _cond.wait();
    }
}

void CountDownLatch::cutDown()
{
    MutexLockGuard lock(_mutex);
    _count--;
    if(_count == 0)
    {
        _cond.notifyAll();
    }
}
