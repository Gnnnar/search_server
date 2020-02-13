#pragma once
#include <functional>
#include <memory>
#include <pthread.h>
#include "Mutex.h"
#include "Condition.h"
#include "CountDownLatch.h"
#include "CurrentThread.h"

class Thread : noncopyable
{
public:
    typedef std::function<void()> ThreadFunc;
    Thread(ThreadFunc func,const std::string& name = std::string());
    ~Thread();
    
    void start();

    int join();

    bool started() const {  return _started;  }

    pid_t tid() const {  return _tid;  }
    
    const std::string & name() const {  return _name;  }

private:
    bool _started;
    bool _joined;
    pthread_t _pthread_id;
    pid_t _tid;
    ThreadFunc _func;
    std::string _name;
    CountDownLatch _latch;
};

