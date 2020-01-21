#pragma once
#include <string>
#include <memory>
#include "noncopyable.h"

class AppendFile;

//对外提供append接口和flush接口，与appendfile不同的是，每当记录日志超过rollSize
//后，会rollfile一个新的appendfile继续记录。
//会对基础基础日志名字自动生成一个详细的名字。

class LogFile : noncopyable
{
public:
    LogFile(const std::string & base_name,off_t roll_size);

    ~LogFile();

    void append(const char* logline,size_t len);

    void flush();

    void rollFile();

private:
    void append_unlocked(const char* logline,size_t len);

    std::string getLogFileName();

    const std::string _basename;
    const off_t _roll_size;
    std::unique_ptr<AppendFile> _file;
};

