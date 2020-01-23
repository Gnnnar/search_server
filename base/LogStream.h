#pragma once
#include <assert.h>
#include <string.h>
#include "FixedBuffer.h"
#include <string>



class LogStream : noncopyable
{
public:
    typedef LogStream self;
    typedef FixedBuffer<kSmallBuffer> Buffer;
    LogStream() {}
    ~LogStream() {}

    self& operator << (bool v)
    {
        _buffer.append(v ? "1":"0",1);
        return *this;
    }

    self& operator<<(short);
    self& operator<<(unsigned short);
    self& operator<<(int);
    self& operator<<(unsigned int);
    self& operator<<(long);
    self& operator<<(unsigned long);
    self& operator<<(long long);
    self& operator<<(unsigned long long);
    self& operator<<(double);

    self& operator<<(float v)
    {
        *this << static_cast<double>(v);
        return *this;
    }

    self& operator << (char v)
    {
        _buffer.append(&v,1);
        return *this;
    }

    self& operator<<(const char* str)
    {
        if (str)
        {
            _buffer.append(str, strlen(str));
        }
        else
        {
            _buffer.append("(null)", 6);
        }
        return *this;
    }

    self& operator<<(const unsigned char* str)
    {
        return operator<<(reinterpret_cast<const char*>(str));
    }

    self& operator<<(const std::string& v)
    {
        _buffer.append(v.c_str(), v.size());
        return *this;
    }

    self& operator<<(const Buffer& v)
    {
        _buffer.append(v.data(),v.length());
        return *this;
    }

    void append(const char* data, int len) {  _buffer.append(data,len); }
    const Buffer & buffer() {  return _buffer; }
    void resetBuffer() {  _buffer.reset(); }

private:
    template<typename T>
        void formatInteger(T);

    Buffer _buffer;
    static const int kMaxNumericSize = 32;
};

