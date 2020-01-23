#include <stdint.h>
#include <stdio.h>
#include <algorithm>
#include <string.h>
#include "LogStream.h"
#include <iostream>

using namespace std;


const char digits[] = "9876543210123456789";
const char* zero = digits + 9;

template <typename T>
size_t convert(char buf[], T value) 
{
    T i = value;
    char* p = buf;
    do {
        int lsd = static_cast<int>(i % 10);
        i /= 10;
        *p++ = zero[lsd];
    } while (i != 0);
    if (value < 0)
    {
        *p++ = '-';
    }
    *p = '\0';
    reverse(buf, p);
    return p - buf;
}

template <typename T>
void LogStream::formatInteger(T v) 
{
    if (_buffer.avail() >= kMaxNumericSize) 
    {
        size_t len = convert(_buffer.current(), v);
        _buffer.add(len);
    }
}

LogStream& LogStream::operator << (short v)
{
    *this << static_cast<int>(v);
    return *this;
}

LogStream& LogStream::operator << (unsigned short v)
{
    *this << static_cast<int>(v);
    return *this;
}

LogStream& LogStream::operator << (int v)
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator << (unsigned int v)
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator << (long v)
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator << (unsigned long v)
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator << (long long v)
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator << (unsigned long long v)
{
    formatInteger(v);
    return *this;
}

LogStream& LogStream::operator << (double v)
{
    if (_buffer.avail() >= kMaxNumericSize) 
    {
        int  len = snprintf(_buffer.current(),kMaxNumericSize,"%.12g",v);
        _buffer.add(len);
    }
    return *this;
}


#if 0

int main()
{
    LogStream os;
    os << 0 << "\n";
    os << 1234567890123 << "\n";
    os << 1.0f << "\n";
    os << 3.1415926 << "\n";
    os << short(2) << "\n";
    os << (long long)5 << "\n";
    os << (unsigned long)5 << "\n";
    os << (unsigned int)5 << "\n";
    os << (unsigned long long)5 << "\n";
    os << 'c' << "\n";
    os << "absdadasd" << "\n";
    os << string("hello word") << "\n";
    
    cout << os.buffer().length() << endl;
    cout << os.buffer().avail() << endl;
    cout << os.buffer().data() << endl;

    return 0;
}

#endif
