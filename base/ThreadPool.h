#pragma once
#include <queue>
#include <vector>
#include <functional>
#include <assert.h>
#include "Mutex.h"
#include "Condition.h"
#include "Thread.h"


class ThreadPool : noncopyable
{
public:
    typedef std::function<void()> Task;
    ThreadPool(const std::string& nameArg = std::string("ThreadPool"));
    ~ThreadPool();
    
    void setMaxQueueSize(int maxSize) {  _capacity = maxSize;  }
    void start(int numThreads);
    void stop();
    
    const std::string& name() const { return _name;  }
    size_t queueSize();
    int  run(Task f);

private:
    void runInThread();
    Task take();

    MutexLock _mutex;
    Condition _cond;
    std::string _name;
    int _capacity;
    bool _running;
    std::vector<std::unique_ptr<Thread>> _threads;
    std::queue<Task> _queue;
};

