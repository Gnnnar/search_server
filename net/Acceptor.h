#pragma once

#include <functional>
#include "../base/noncopyable.h"
#include "Channel.h"
#include "Socket.h"

class EventLoop;
class InetAddress;

class Acceptor : noncopyable
{
public:
    typedef std::function<void (int sockfd,const InetAddress&)> NewConnectionCallback;

    Acceptor(EventLoop* loop, const InetAddress& listenAddr);

    void setNewConnectionCallback(const NewConnectionCallback& cb)
    { _newConnectionCallback = cb;  }

    bool listenning() const { return _listenning;  }

    void listen();

private:
    void handleRead();

    EventLoop* _loop;
    Socket _acceptSocket;
    Channel _acceptChannel;
    NewConnectionCallback _newConnectionCallback;
    bool _listenning;
};

