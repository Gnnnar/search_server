#pragma once
#include "Callbacks.h"
#include "InetAddress.h"
#include "Buffer.h"
#include "../base/noncopyable.h"

class Channel;
class EventLoop;
class Socket;



class TcpConnection
: noncopyable
    ,public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop* loop,
                  const std::string& name,
                  int sockfd,
                  const InetAddress& localAddr,
                  const InetAddress& peerAddr);
    ~TcpConnection();
    EventLoop* getLoop() const { return _loop;  }
    const std::string& name() const { return _name;  }
    const InetAddress& localAddress() { return _localAddr;  }
    const InetAddress& peerAddress() { return _peerAddr;  }
    bool connected() const { return _state == kConnected;  }
    
    void send(const std::string& message);

    void shutdown();

    void setConnectionCallback(const ConnectionCallback& cb)
    { _connectionCallback = cb;  }

    void setMessageCallback(const MessageCallback& cb)
    { _messageCallback = cb;  }
    
    void setCloseCallback(const CloseCallback& cb)
    { _closeCallback = cb;  }

    void connectEstablished();

    void connectDestroyed();

private:
    enum StateE { kConnecting, kConnected, kDisconnecting, kDisconnected };
    void setState(StateE s) { _state = s;  };
    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();
    
    void sendInLoop(const std::string& message);
    void shutdownInLoop();

    EventLoop *_loop;
    std::string _name;
    StateE _state;
    std::unique_ptr<Socket> _socket;
    std::unique_ptr<Channel> _channel;
    const InetAddress _localAddr;
    const InetAddress _peerAddr;
    ConnectionCallback _connectionCallback;
    MessageCallback _messageCallback;
    CloseCallback _closeCallback;
    Buffer _inputBuffer;
    Buffer _outputBuffer;
};

