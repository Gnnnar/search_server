#pragma once
#include <assert.h>
#include <string.h>
#include "noncopyable.h"


const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000*1000;

template<int SIZE>
class FixedBuffer : noncopyable
{
public:
    FixedBuffer() : _cur(_data) {}
    ~FixedBuffer() {}

    void append(const char* buf,size_t len)
    {
        if(avail() > (int)len)
        {
            memcpy(_cur,buf,len);
            _cur += len;
        }
    }
    
    int avail() const {  return int(this->end() - _cur);  }

    char * current() {  return _cur;  }

    const char* data() const {  return _data;  }

    int length() const {  return _cur - _data;  }

    void add(size_t len) {  _cur += len;  }

    void reset() {  _cur = _data;  }

    void bzero() {  memset(_data,0,sizeof(_data));  }

private:
    const char* end() const {  return _data + sizeof(_data);  }

    char _data[SIZE];
    char *_cur;
};

