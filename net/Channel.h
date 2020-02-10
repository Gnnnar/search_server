#ifndef MUDUO_NET_CHANNEL_H
#define MUDUO_NET_CHANNEL_H

#include <functional>
#include <poll.h>
#include "../base/noncopyable.h"
#include "../base/Timestamp.h"

class EventLoop;

class Channel : noncopyable 
{
public:
    typedef std::function<void()> EventCallback;
    typedef std::function<void(Timestamp)> ReadEventCallback;

    Channel(EventLoop* loop,int fd);
    ~Channel();

    void handleEvent(Timestamp receiveTime);
    void setReadCallback(const ReadEventCallback& cb)
    { _readCallback = cb;  }
    void setWriteCallback(const EventCallback& cb)
    { _writeCallback = cb;  }
    void setErrorCallback(const EventCallback& cb)
    { _errorCallback = cb;  }
    void setCloseCallback(const EventCallback& cb)
    { _closeCallback = cb;  }

    int fd() const { return _fd;  }
    int events() const { return _events;  }
    void set_revents(int revt) { _revents = revt;  }
    bool isNoneEvent() const { return _events == kNoneEvent;  }
    
    void enableReading() { _events |= kReadEvent; update();  }

    void enableWriting() { _events |= kWriteEvent; update();  }
    void disableWriting() { _events &= ~kWriteEvent; update();  }

    void disableAll() { _events = kNoneEvent; update();  }
    
    bool isWriting() const { return _events & kWriteEvent;  }

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

    bool _eventHandling;

    ReadEventCallback _readCallback;
    EventCallback _writeCallback;
    EventCallback _errorCallback;
    EventCallback _closeCallback;
};

#endif
