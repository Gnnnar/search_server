#pragma once
#include <string>
#include "noncopyable.h"


//真正打开文件，向文件里写日志的类，最为底层的操作，自定义了一个64K的缓冲区。

class AppendFile : noncopyable
{
public:
    AppendFile(const std::string & file_name);

    ~AppendFile();
    
    void append(const char* logline,const size_t len);

    void flush();

    off_t writtenBytes() const {    return _written_bytes;  }

private:
    size_t write(const char* logline,const size_t len);

    FILE *_fp;
    char _buffer[64*1024];
    off_t _written_bytes;
};

