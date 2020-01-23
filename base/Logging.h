#pragma once
#include "FixedBuffer.h"
#include "LogStream.h"


class Logger
{
public:
    enum LogLevel
    {
        TRACE,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL,
        NUM_LOG_LEVELS,
    };

    Logger(const char *fileName, int line);
    Logger(const char *fileName, int line,LogLevel level);
    Logger(const char *fileName, int line,LogLevel level,const char *func);
    Logger(const char* fileName, int line, bool toAbort);
    ~Logger();

    LogStream& stream() {  return  _impl._stream; }
    static LogLevel getlogLevel();
    static void setlogLevel(LogLevel);


private:
    class Impl
    {
    public:
        typedef Logger::LogLevel LogLevel;
        Impl(LogLevel level,int old_errno,const char* fileName,int line);
        void formatTime();
        void finish();
        LogStream _stream;
        LogLevel _level;
        std::string _fileName;
        int _line;
    };

    Impl _impl;

};

extern Logger::LogLevel g_logLevel;
Logger::LogLevel Logger::getlogLevel()
{
    return g_logLevel;
}


#define LOG_TRACE if (Logger::getlogLevel() <= Logger::TRACE) \
                                                   Logger(__FILE__, __LINE__, Logger::TRACE, __func__).stream()
#define LOG_DEBUG if (Logger::getlogLevel() <= Logger::DEBUG) \
                                                   Logger(__FILE__, __LINE__, Logger::DEBUG, __func__).stream()
#define LOG_INFO if (Logger::getlogLevel() <= Logger::INFO) \
                                                  Logger(__FILE__, __LINE__).stream()
#define LOG_WARN Logger(__FILE__, __LINE__, Logger::WARN).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, false).stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, true).stream()

