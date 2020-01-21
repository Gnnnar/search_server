#pragma once
#include <memory>
#include <vector>
#include <string>
#include "Mutex.h"
#include "Thread.h"
#include "noncopyable.h"
#include "FixedBuffer.h"

class AsyncLogging
{
public:
    AsyncLogging(const std::string &basename,off_t roll_size,int flush_interval = 3 );
    ~AsyncLogging()
    {
        if(_running)
        {
            stop();
        }
    }
 
    void append(const char* logline, int len);

    void start()
    {
        _running = true;
        _thread.start();
    }

    void stop()
    {
        _running = false;
        _cond.notifyAll();
        _thread.join();
    }

private:
    void threadFunc();

    typedef FixedBuffer<kLargeBuffer> Buffer;
    typedef std::unique_ptr<Buffer> BufferPtr;
    typedef std::vector<BufferPtr> BufferVector;

    const int _flush_interval;
    bool _running;
    const std::string _basename;
    const off_t _roll_size;
    Thread _thread;
    MutexLock _mutex;
    Condition _cond;
    BufferPtr _current_buffer;
    BufferPtr _next_buffer;
    BufferVector _buffers;
};

