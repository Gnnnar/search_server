#include <iostream>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sstream>
#include <sys/time.h>
#include <unistd.h>
#include "AsyncLogging.h"
#include "Logging.h"
#include "CurrentThread.h"

using namespace std;

static pthread_once_t once_control_ = PTHREAD_ONCE_INIT;
static AsyncLogging *AsyncLogger_ = nullptr;
Logger::LogLevel  Logger::g_logLevel = Logger::TRACE;

void once_init()//应该在单例的配置文件类获取参数
{
    AsyncLogger_ = new AsyncLogging("../log/SeServer",64*1024*1024);
    AsyncLogger_->start(); 
}

void output(const char* msg, int len)
{
    pthread_once(&once_control_, once_init);
    AsyncLogger_->append(msg, len);
}

__thread char t_errnobuf[512];
__thread char t_time[64];
__thread time_t t_lastSecond;

const char* LogLevelName[Logger::NUM_LOG_LEVELS] =
{
    "TRACE ",
    "DEBUG ",
    "INFO  ",
    "WARN  ",
    "ERROR ",
    "FATAL ",
};

const char* strerror_tl(int savedErrno)
{
    return strerror_r(savedErrno, t_errnobuf, sizeof t_errnobuf);
}

Logger::Impl::Impl(LogLevel level,int old_errno,const char* fileName,int line)
    : _stream()
    , _level(level)
    , _fileName(fileName)
      , _line(line)
{
    formatTime();
    CurrentThread::tid();
    _stream << CurrentThread::name() << ':';
    _stream.append(CurrentThread::tidString(),CurrentThread::tidStringLength());
    _stream.append(LogLevelName[level],6);
    if(old_errno != 0)
    {
        _stream << strerror_tl(old_errno) <<  " (errno=" <<old_errno << ") ";
    }
}

void Logger::Impl::formatTime()
{
    struct timeval tv;
    struct tm tm_time;
    gettimeofday(&tv,NULL);
    if(t_lastSecond != tv.tv_sec)
    {
        localtime_r(&tv.tv_sec,&tm_time);
        int len = snprintf(t_time, sizeof(t_time), "%4d%02d%02d %02d:%02d:%02d.",
                       tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                       tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
        assert(len == 18);
        len = 0;

    }
    _stream.append(t_time,18);
    char buf[10];
    int len = snprintf(buf,sizeof buf,"%6d ",(int)tv.tv_usec);
    _stream.append(buf,len);
}

void Logger::Impl::finish()
{
    _stream << " - " << _fileName << ':' << _line << '\n';
}

Logger::Logger(const char *fileName, int line)
:_impl(INFO,0,fileName,line)
{}

Logger::Logger(const char *fileName, int line,LogLevel level)
:_impl(level,0,fileName,line)
{}

Logger::Logger(const char *fileName, int line,LogLevel level,const char* func)
:_impl(level,0,fileName,line)
{
    _impl._stream << func << ' ';
}

Logger::Logger(const char* fileName, int line, bool toAbort)
: _impl(toAbort?FATAL:ERROR, errno, fileName, line)
{}

Logger::~Logger()
{
    _impl.finish();
    const LogStream::Buffer& buf(_impl._stream.buffer()); 
    output(buf.data(),buf.length());
    if(_impl._level  == FATAL)
    {
        AsyncLogger_->stop();
        abort();
    }
}

void Logger::setlogLevel(Logger::LogLevel level)
{
    g_logLevel = level;
}

#if 0

int main()
{
    errno = 5;
    LOG_INFO << "hello word";
    sleep(1);
    LOG_FATAL << "hello word";
    return 0;
}

#endif
