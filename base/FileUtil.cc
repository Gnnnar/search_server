#include "FileUtil.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

AppendFile::AppendFile(const std::string & file_name)
: _fp(fopen(file_name.c_str(),"ae"))
, _written_bytes(0)
{
    setbuffer(_fp,_buffer,sizeof(_buffer));
}

AppendFile::~AppendFile()
{
    fclose(_fp);
}

void AppendFile::append(const char* logline,const size_t len)
{
    size_t n = write(logline,len);
    size_t remain = len - n;
    while(remain > 0)
    {
        size_t x = write(logline + n,remain);
        if(x == 0)
        {
            int err = ferror(_fp);
            if(err) 
                fprintf(stderr, "AppendFile::append() failed !\n");
            break;
        }
        n += x;
        remain = len - n;
    }
    _written_bytes += len;
}

void AppendFile::flush()
{
    fflush(_fp);
}

size_t AppendFile::write(const char* logline,const size_t len)
{
    return fwrite_unlocked(logline,1,len,_fp);
}

#if 0

int main()
{
    AppendFile ap("test.log");
    ap.append("hello\nword\n",11);
    ap.flush();
    printf("%d\n",(int)ap.writtenBytes());
    return 0;
}

#endif

