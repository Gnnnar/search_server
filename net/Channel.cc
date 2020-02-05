#include <iostream>

#include "Channel.h"
#include "EventLoop.h"
#include "../base/Logging.h"

Channel::Channel(EventLoop* loop,int fd)
    : _loop(loop)
    , _fd(fd)
    , _events(0)
    , _revents(0)
      , _index(-1)
{}

void Channel::update()
{
    _loop->updateChannel(this);
}

void Channel::handleEvent()
{
    if (_revents & POLLNVAL) {
        LOG_WARN << "Channel::handle_event() POLLNVAL";
    }
    if (_revents & (POLLERR | POLLNVAL)) {
        if (_errorCallback) _errorCallback();
    }
    if (_revents & (POLLIN | POLLPRI | POLLRDHUP)) {
        if (_readCallback) _readCallback();
    }
    if (_revents & POLLOUT) {
        if (_writeCallback) _writeCallback();
    }
}

