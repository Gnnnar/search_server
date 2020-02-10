#pragma once
#include "../base/noncopyable.h"

class InetAddress;

class Socket
{
public:
    Socket(int sockfd)
    : _sockfd(sockfd)
    {}
    ~Socket();
    
    int fd() const {  return _sockfd; }

    void bindAddress(const InetAddress& localaddr);

    void listen();

    int accept(InetAddress* peeraddr);

    void setReuseAddr(bool on);

    void shutdownWrite();

private:
    const int _sockfd;
};

