#include "TcpConnection.h"
#include <unistd.h>
#include "../base/Logging.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"

TcpConnection::TcpConnection(EventLoop* loop,
                             const std::string& nameArg,
                             int sockfd,
                             const InetAddress& localAddr,
                             const InetAddress& peerAddr)
    : _loop(loop)
    , _name(nameArg)
    , _state(kConnecting)
    , _socket(new Socket(sockfd))
    , _channel(new Channel(loop,sockfd))
    , _localAddr(localAddr)
      , _peerAddr(peerAddr)
{
    LOG_DEBUG << "TcpConnection::ctor[" <<  _name << "] at "  << " fd=" << sockfd;
    _channel->setReadCallback(bind(&TcpConnection::handleRead, this));
    _channel->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    _channel->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    _channel->setErrorCallback(std::bind(&TcpConnection::handleError, this));
}

TcpConnection::~TcpConnection()
{
    LOG_DEBUG << "TcpConnection::dtor[" <<  _name << "] at "<< " fd=" << _channel->fd();
}

void TcpConnection::connectEstablished()
{
    _loop->assertInLoopThread();
    assert(_state == kConnecting);
    setState(kConnected);
    _channel->enableReading();
    _connectionCallback(shared_from_this());
}

void TcpConnection::connectDestroyed()
{
    _loop->assertInLoopThread();
    assert(_state == kConnected);
    setState(kDisconnected);
    _channel->disableAll();
    _connectionCallback(shared_from_this());
    _loop->removeChannel(_channel.get());
}

void TcpConnection::handleRead()
{
    char buf[65536];
    ssize_t n = read(_channel->fd(), buf, sizeof buf);
    if (n > 0) {
        _messageCallback(shared_from_this(), buf, n);
    } else if (n == 0) {
        handleClose();
    } else {
        handleError();
    }
}

void TcpConnection::handleWrite()
{
}

void TcpConnection::handleClose()
{
    _loop->assertInLoopThread();
    LOG_TRACE << "TcpConnection::handleClose state = " << _state;
    assert(_state == kConnected);
    _channel->disableAll();
    _closeCallback(shared_from_this());
}

void TcpConnection::handleError()
{
    LOG_ERROR << "TcpConnection::handleError [" << _name
        << "] - SO_ERROR = " << errno << " " << strerror(errno);
}

