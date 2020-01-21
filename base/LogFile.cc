#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <iostream>
#include <string.h>
#include "FileUtil.h"
#include "LogFile.h"


LogFile::LogFile(const std::string & base_name,off_t roll_size)
    : _basename(base_name)
      , _roll_size(roll_size)
{
    rollFile();
}

LogFile::~LogFile() {}

void LogFile::append(const char* logline,size_t len)
{
    append_unlocked(logline,len);
}

void LogFile::flush()
{
    _file->flush();
}

void LogFile::rollFile()
{
    std::string name = getLogFileName();
    _file.reset(new AppendFile(name));
}

void LogFile::append_unlocked(const char* logline,size_t len)
{
    _file->append(logline,len);
    if(_file->writtenBytes() > _roll_size)
    {
        rollFile();
    }
}

std::string LogFile::getLogFileName()
{
    std::string filename(_basename);
    time_t now = time(NULL);
    char time_buf[32];
    struct tm tm;
    localtime_r(&now,&tm);
    strftime(time_buf, sizeof(time_buf), ".%Y%m%d-%H%M%S.", &tm);
    filename += time_buf;
    char hostname[256] = "host@thy";
    /*if (gethostname(hostname, sizeof hostname) == 0)
    {
        hostname[sizeof(hostname)-1] = '\0';
    }
    else
    {
        strcpy(hostname,"unknownhost");
    }*/
    filename += hostname;
    char pid_buf[40];
    sprintf(pid_buf,".%d.log",getpid()); 
    filename += pid_buf;
    return filename;
}


#if 0

int main()
{
    LogFile log("test",10);
    for(int i=0;i<9;i++)
    {
        std::string tmp(2,'a'+i);
        tmp += "\n";
        log.append(tmp.c_str(),tmp.size());
        sleep(1);
        if((i+1) % 3 == 0)
        {
            std::cout << "flush! i = " << i << std::endl;
            log.flush();
        }
    }
    return 0;
}

#endif
