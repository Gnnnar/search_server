#include "Thread.h"
#include <assert.h>
#include <errno.h>
#include <linux/unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <memory>
#include <iostream>
#include "CurrentThread.h"

using namespace std;

namespace CurrentThread
{

__thread int t_cachedTid = 0;
__thread char t_tidString[32];
__thread int t_tidStringLength = 6;
__thread const char* t_threadName = "default";

}

pid_t gettid() 
{ 
    return static_cast<pid_t>(::syscall(SYS_gettid)); 
}

void CurrentThread::cacheTid() 
{
    if (t_cachedTid == 0) 
    {
        t_cachedTid = gettid();
        t_tidStringLength =
            snprintf(t_tidString, sizeof t_tidString, "%5d ", t_cachedTid);
    }
}

bool CurrentThread::isMainThread()
{
      return tid() == ::getpid();
}

class ThreadNameInitializer
{
public:
    ThreadNameInitializer()
    {
        CurrentThread::t_threadName = "main";
        CurrentThread::tid();
    }
};

ThreadNameInitializer init;

//代理间接创建线程
struct ThreadData
{
    Thread::ThreadFunc _func;
    string _name;
    pid_t* _tid;
    CountDownLatch* _latch;

    ThreadData(Thread::ThreadFunc func,
               const string& name,
               pid_t* tid,
               CountDownLatch* latch)
        : _func(move(func))
          , _name(name)
          , _tid(tid)
          , _latch(latch)
    {}

    void runInThread()
    {
        *_tid = CurrentThread::tid();
        CurrentThread::t_threadName = _name.c_str();
        _latch->cutDown();
        _func();
    }
};

void* startThread(void *obj)
{
    ThreadData* data = static_cast<ThreadData*>(obj);
    data->runInThread();
    delete data;
    return NULL;
}

Thread::Thread(ThreadFunc func,const std::string& name)
    : _started(false)
    , _joined(false)
    , _pthread_id(0)
    , _tid(0)
    , _func(std::move(func))
    , _name(name)
      , _latch(1)
{
    if(_name.empty())
    {
        _name = string("Thread");
    }
}

Thread::~Thread()
{
    if (_started&& _joined)
    {
        pthread_detach(_pthread_id);
    }
}

void Thread::start()
{
    _started = true;
    ThreadData *data = new ThreadData(_func,_name,&_tid,&_latch);
    pthread_create(&_pthread_id,NULL,startThread,data);
    _latch.wait();
}

int Thread::join() 
{
    assert(_started);
    assert(!_joined);
    _joined = true;
    return pthread_join(_pthread_id, NULL);
}



#if 0

void test()
{
    cout << CurrentThread::isMainThread() << endl;
    cout << CurrentThread::tid() <<endl;
    cout << CurrentThread::name() << endl;
}


int main()
{
    test();
    Thread td(test,"sonThread");
    td.start();
    td.join();
}

#endif



