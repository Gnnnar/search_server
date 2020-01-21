#pragma once
#include <time.h>
#include <pthread.h>
#include "Mutex.h"



class Condition
{
public:
    Condition(MutexLock & mutex) : _mutex(mutex) {  pthread_cond_init(&_cond,NULL);  }
    ~Condition() {  pthread_cond_destroy(&_cond); }

    void wait() {  pthread_cond_wait(&_cond,_mutex.get()); }

    void notify() {  pthread_cond_signal(&_cond); }

    void notifyAll() {  pthread_cond_broadcast(&_cond); }

    void waitForSeconds(int seconds) {
        struct timespec abstime;
        clock_gettime(CLOCK_REALTIME, &abstime);
        abstime.tv_sec += static_cast<time_t>(seconds);
        pthread_cond_timedwait(&_cond, _mutex.get(), &abstime);
    }

private:
    MutexLock & _mutex;
    pthread_cond_t _cond;
};

