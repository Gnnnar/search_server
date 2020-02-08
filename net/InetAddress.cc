#include "InetAddress.h"
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

InetAddress::InetAddress(uint16_t port)
{
    bzero(&_addr,sizeof _addr);
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    _addr.sin_port = htons(port);
}

InetAddress::InetAddress(const std::string& ip, uint16_t port)
{
    bzero(&_addr,sizeof _addr);
    _addr.sin_family = AF_INET;
    _addr.sin_addr.s_addr = inet_addr(ip.c_str());
    _addr.sin_port = htons(port);
}

std::string InetAddress::toHostPort() const
{
    char host[INET_ADDRSTRLEN] = "INVALID";
    char buf[32];
    ::inet_ntop(AF_INET, &_addr.sin_addr, host, sizeof host);
    uint16_t port = ntohs(_addr.sin_port);
    snprintf(buf, 32, "%s:%u", host, port);
    return buf;
}
