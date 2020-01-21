
#include <unistd.h>
#include <iostream>
#include "AsyncLogging.h"
#include "LogFile.h"
#include "CurrentThread.h"

using namespace std;

AsyncLogging::AsyncLogging(const std::string &basename,off_t roll_size,int flush_interval)
: _flush_interval(flush_interval)
, _running(false)
, _basename(basename)
, _roll_size(roll_size)
, _thread(bind(&AsyncLogging::threadFunc,this),"logging")
, _cond(_mutex)
, _current_buffer(new Buffer)
, _next_buffer(new Buffer)
{
    _current_buffer->bzero();
    _next_buffer->bzero();
    _buffers.reserve(16);
}

//前端
void AsyncLogging::append(const char* logline, int len)
{
    MutexLockGuard lock(_mutex);
    if(_current_buffer->avail() > len)
    {
        _current_buffer->append(logline,len);
    }
    else
    {
        _buffers.push_back(std::move(_current_buffer));
        if(_next_buffer != nullptr)
        {
            _current_buffer = std::move(_next_buffer);
        }
        else
        {
            _current_buffer.reset(new Buffer);
        }
        _current_buffer->append(logline,len);
        _cond.notify();
    }
}

//后端
void AsyncLogging::threadFunc()
{
    LogFile output(_basename,_roll_size);
    BufferPtr new_buffer1(new Buffer);
    BufferPtr new_buffer2(new Buffer);
    BufferVector buffers_to_write;
    new_buffer1->bzero();
    new_buffer2->bzero();
    buffers_to_write.reserve(16);
    while(_running)
    {
        assert(new_buffer1 && new_buffer1->length() == 0);
        assert(new_buffer2 && new_buffer2->length() == 0);
        assert(buffers_to_write.empty());
        //交换，把前端的两块缓冲区填满，要临界区尽可能短
        {
            MutexLockGuard lock(_mutex);
            if(_buffers.empty())//不能用while，因为3s内可能没写满也要写入磁盘
            {
                _cond.waitForSeconds(_flush_interval);
            }

#if 0
            if(_buffers.empty())
            {
                cout << "3s刷新" << endl;
                cout << _current_buffer->length() << endl;
            }else
            {
                cout << "满了刷新" << endl;
                cout << _buffers.back()->length() << endl;
            }

#endif


            _buffers.push_back(std::move(_current_buffer));
            _buffers.swap(buffers_to_write);
            _current_buffer = std::move(new_buffer1);
            if(_next_buffer == nullptr)
            {
                _next_buffer = std::move(new_buffer2);
            }
        }
        
        assert(!buffers_to_write.empty());
        if(buffers_to_write.size() > 25)
        {
            char buf[256];
            time_t now(time(NULL));
            snprintf(buf,sizeof(buf),"Dropped log messages at %s, %d larger buffers\n"
                     ,ctime(&now),(int)buffers_to_write.size()-2);
            fputs(buf, stderr);
            output.append(buf,strlen(buf));
            buffers_to_write.erase(buffers_to_write.begin()+2,buffers_to_write.end());
        }

        for(const auto& buffer : buffers_to_write)
        {
            output.append(buffer->data(),buffer->length());
        }

        if(buffers_to_write.size() > 2)
        {
            buffers_to_write.resize(2);
        }

        if(!new_buffer1)//必然给cur了
        {
            assert(!buffers_to_write.empty());
            new_buffer1 = (std::move(buffers_to_write.back()));
            new_buffer1->reset();
            buffers_to_write.pop_back();
        }
        if(!new_buffer2)//如果被用，则next为空，buffers的size至少为2;
        {
            assert(!buffers_to_write.empty());
            new_buffer2 = (std::move(buffers_to_write.back()));
            new_buffer2->reset();
            buffers_to_write.pop_back();
        }
        
        buffers_to_write.clear();
        output.flush();
    }
    output.flush();
}


#if 0

AsyncLogging as("test",8*1024*1024);


void print()
{
    for(int i = 0;i < 499999;i++)
    {   //usleep(1);
        char buf[200];
        sprintf(buf,"%s:%d--%d\n",CurrentThread::name(),CurrentThread::tid(),i);
        as.append(buf,strlen(buf));
    }
}

int main()
{
    as.start();
    Thread td1(print,"aaaaaa");
    Thread td2(print,"bbbbbb");
    Thread td3(print,"cccccc");
    td1.start();
    td2.start();
    td3.start();
    print();
    td1.join();
    td2.join();
    td3.join();
    as.stop();
}


#endif














