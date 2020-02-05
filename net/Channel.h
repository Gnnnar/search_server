#pragma once
#include <functional>
#include <poll.h>
#include "../base/noncopyable.h"

class EventLoop;

class Channel : noncopyable 
{
public:
    typedef std::function<void()> EventCallback;
    Channel(EventLoop* loop,int fd);

    void handleEvent();
    void setReadCallback(const EventCallback& cb)
    { _readCallback = cb;  }
    void setWriteCallback(const EventCallback& cb)
    { _writeCallback = cb;  }
    void setErrorCallback(const EventCallback& cb)
    { _errorCallback = cb;  }

    int fd() const { return _fd;  }
    int events() const { return _events;  }
    void set_revents(int revt) { _revents = revt;  }
    bool isNoneEvent() const { return _events == kNoneEvent;  }

    void enableReading() { _events |= kReadEvent; update();  }

    int index() { return _index;  }
    void set_index(int idx) { _index = idx;  }

    EventLoop* ownerLoop() { return _loop;  }


private:
    void update();

    static const int kNoneEvent = 0;
    static const int kReadEvent = POLLIN | POLLPRI;
    static const int kWriteEvent = POLLOUT;

    EventLoop *_loop;
    const int _fd;
    int _events;
    int _revents;
    int _index;
    EventCallback _readCallback;
    EventCallback _writeCallback;
    EventCallback _errorCallback;
};

