#include "TcpConnection.h"
#include <unistd.h>
#include "../base/Logging.h"
#include "Channel.h"
#include "EventLoop.h"
#include "Socket.h"

using namespace std::placeholders; 

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
    _channel->setReadCallback(bind(&TcpConnection::handleRead, this,_1));
    _channel->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    _channel->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    _channel->setErrorCallback(std::bind(&TcpConnection::handleError, this));
}

TcpConnection::~TcpConnection()
{
    LOG_DEBUG << "TcpConnection::dtor[" <<  _name << "] at "<< " fd=" << _channel->fd();
}

void TcpConnection::send(const std::string& message)
{
    if (_state == kConnected) {
        if (_loop->isInLoopThread()) {
            sendInLoop(message);
        } else {
            _loop->runInLoop(std::bind(&TcpConnection::sendInLoop, this, message));
        }
    }
}

void TcpConnection::shutdown()
{
    if (_state == kConnected)
    {
        setState(kDisconnecting);
    }
    _loop->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
}

void TcpConnection::sendInLoop(const std::string& message)
{
    _loop->assertInLoopThread();
    ssize_t nwrote = 0;
    if (!_channel->isWriting() && _outputBuffer.readableBytes() == 0) 
    {
        nwrote = ::write(_channel->fd(), message.data(), message.size());
        if(nwrote >= 0)
        {
            if((size_t)nwrote < message.size())
            {
                LOG_TRACE << "I am going to write more data";
            }
        }else
        {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                LOG_ERROR << "TcpConnection::sendInLoop";
            }
        }
    }
    assert(nwrote >= 0);
    if((size_t)nwrote < message.size())
    {
        _outputBuffer.append(message.data()+nwrote,message.size()-nwrote);
        if (!_channel->isWriting()) {
            _channel->enableWriting();
        }
    }
}

void TcpConnection::shutdownInLoop()
{
    _loop->assertInLoopThread();
    if(!_channel->isWriting())
    {
        _socket->shutdownWrite();
    }
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
    assert(_state == kConnected || _state == kDisconnecting);
    setState(kDisconnected);
    _channel->disableAll();
    _connectionCallback(shared_from_this());
    _loop->removeChannel(_channel.get());
}

void TcpConnection::handleRead(Timestamp receiveTime)
{
    int savedErrno = 0;
    ssize_t n = _inputBuffer.readFd(_channel->fd(),&savedErrno);
    if (n > 0) {
        _messageCallback(shared_from_this(),&_inputBuffer,receiveTime);
    } else if (n == 0) {
        handleClose();
    } else {
        errno = savedErrno;
        LOG_ERROR << "TcpConnection::handleRead";
        handleError();
    }
}

void TcpConnection::handleWrite()
{
    _loop->assertInLoopThread();
    if (_channel->isWriting())
    {
        ssize_t n = ::write(_channel->fd(),_outputBuffer.peek(),_outputBuffer.readableBytes());
        if(n > 0)
        {
            _outputBuffer.retrieve(n);
            if(_outputBuffer.readableBytes() == 0)
            {
                _channel->disableWriting();
                if(_state == kDisconnecting)
                {
                    _socket->shutdownWrite();
                }
            }else
            {
                LOG_TRACE << "I am going to write more data";
            }
        }
        else
        {
            LOG_ERROR << "TcpConnection::handleWrite";
        }
    }else
    {
        LOG_TRACE << "Connection is down, no more writing";
    }
}

void TcpConnection::handleClose()
{
    _loop->assertInLoopThread();
    LOG_TRACE << "TcpConnection::handleClose state = " << _state;
    assert(_state == kConnected || _state == kDisconnecting);
    _channel->disableAll();
    _closeCallback(shared_from_this());
}

void TcpConnection::handleError()
{
    LOG_ERROR << "TcpConnection::handleError [" << _name
        << "] - SO_ERROR = " << errno << " " << strerror(errno);
}

