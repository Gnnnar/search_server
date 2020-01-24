#include <iostream>
#include <unistd.h>
#include "ThreadPool.h"
#include "Mutex.h"
#include "Logging.h"
using namespace std;

ThreadPool::ThreadPool(const std::string& nameArg)
    : _cond(_mutex)
    , _name(nameArg)
    , _capacity(0)
      , _running(false)
{}

ThreadPool::~ThreadPool()
{ 
    if(_running)
        stop();
} 

void ThreadPool::start(int numThreads)
{
    assert(_threads.empty());
    _running = true;
    _threads.reserve(numThreads);
    for(int i = 0; i < numThreads; ++i)
    {
        char id[32];
        snprintf(id, sizeof id, "%d", i+1);
        _threads.emplace_back(new Thread(bind(&ThreadPool::runInThread,this),_name+id)); 
        _threads[i]->start();
    }                        
}                     

void ThreadPool::stop()
{
    {
    MutexLockGuard lock(_mutex);
    _running = false;
    _cond.notifyAll();
    }
    for (auto& thr : _threads)
    {
        thr->join();
    }
}

size_t ThreadPool::queueSize()
{
    MutexLockGuard lock(_mutex);
    return _queue.size();
}

int ThreadPool::run(Task task)
{
    MutexLockGuard lock(_mutex);
    if(_queue.size() >= (size_t)_capacity)
        return -1;
    _queue.push(std::move(task));
    _cond.notify();
    return 0;
}

ThreadPool::Task ThreadPool::take()
{
    MutexLockGuard lock(_mutex);
    while(_queue.empty() && _running)
    {
        _cond.wait();
    }
    Task task;
    if (!_queue.empty())
    {
        task = _queue.front();
        _queue.pop();
    }
    return task;
}

void ThreadPool::runInThread()
{
    while (_running)
    {
        Task task(take());
        if(task)
        {
            task();
        }
    }
}

#if 0

void test(int i)
{
    LOG_INFO << i;
    usleep(50);
}

int main()
{
    ThreadPool tp("test");
    tp.setMaxQueueSize(1000);
    tp.start(3);
    sleep(1);
    for(int i = 0;i < 2000;i++)
    {
        if(tp.run(bind(test,i)) == -1)
            cout << i << ":override" << endl;
    }
    sleep(1);
    tp.stop();
    LOG_FATAL << "exit";
    return 0;
}
#endif
