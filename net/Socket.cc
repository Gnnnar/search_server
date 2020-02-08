#include "Socket.h"
#include "InetAddress.h"
#include "SocketsOps.h"
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <strings.h> 

Socket::~Socket()
{
    sockets::close(_sockfd);
}

void Socket::bindAddress(const InetAddress& addr)
{
    sockets::bindOrDie(_sockfd, addr.getSockAddrInet());
}

void Socket::listen()
{
    sockets::listenOrDie(_sockfd);
}

int Socket::accept(InetAddress* peeraddr)
{
    struct sockaddr_in addr;
    bzero(&addr, sizeof addr);
    int connfd = sockets::accept(_sockfd, &addr);
    if (connfd >= 0)
    {
        peeraddr->setSockAddrInet(addr);
    }
    return connfd;
}

void Socket::setReuseAddr(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR,
                 &optval, sizeof optval);
}
