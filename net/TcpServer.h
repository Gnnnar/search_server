#pragma once

#include "Callbacks.h"
#include "TcpConnection.h"
#include <map>


class Acceptor;
class EventLoop;

class TcpServer : noncopyable
{
public:
    TcpServer(EventLoop* loop, const InetAddress& listenAddr);
    ~TcpServer(); 
    
    void start();

    void setConnectionCallback(const ConnectionCallback& cb)
          { _connectionCallback = cb;  }
    
    void setMessageCallback(const MessageCallback& cb)
          { _messageCallback = cb;  }
    


private:
    void newConnection(int sockfd, const InetAddress& peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);

    typedef std::map<std::string, TcpConnectionPtr> ConnectionMap;

    EventLoop* _loop;
    std::string _name;
    std::unique_ptr<Acceptor> _acceptor;
    ConnectionCallback _connectionCallback;
    MessageCallback _messageCallback;
    bool _started;
    int _nextConnId;
    ConnectionMap _connections;
};

