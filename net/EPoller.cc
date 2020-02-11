#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>
#include "EPoller.h"
#include "Channel.h"
#include "../base/Logging.h"

namespace
{
const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;
}

EPoller::EPoller(EventLoop* loop)
    : _ownerLoop(loop)
    , _epollfd(epoll_create1(EPOLL_CLOEXEC))
      , _events(kInitEventListSize)
{
    if (_epollfd < 0)
    {
        LOG_FATAL << "EPollPoller::EPollPoller";
    }
}

EPoller::~EPoller()
{
    close(_epollfd);
}

Timestamp EPoller::poll(int timeoutMs, ChannelList* activeChannels)
{
    int numEvents = ::epoll_wait(_epollfd,&*_events.begin(),
                                 static_cast<int>(_events.size()),timeoutMs);
    Timestamp now(Timestamp::now());
    if(numEvents > 0)
    {
        LOG_TRACE << numEvents << " events happended";
        fillActiveChannels(numEvents, activeChannels);
        if((size_t)numEvents == _events.size())
        {
            _events.resize(2 * _events.size());
        }
    }else if(numEvents == 0)
    {
        LOG_TRACE << " nothing happended";
    }else
    {
        LOG_ERROR << "EPoller::poll()";
    }
    return now;
}

void EPoller::fillActiveChannels(int numEvents,
                                 ChannelList* activeChannels) const
{
    assert((size_t)numEvents <= _events.size());
    for(int i = 0; i < numEvents; ++i)
    {
        Channel* channel = static_cast<Channel*>(_events[i].data.ptr);
#ifndef NDEBUG
        int fd = channel->fd();
        ChannelMap::const_iterator it = _channels.find(fd);
        assert(it != _channels.end());
        assert(it->second == channel);
#endif
        channel->set_revents(_events[i].events);
        activeChannels->push_back(channel);
    }
}

void EPoller::updateChannel(Channel* channel)
{
    assertInLoopThread();
    LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();
    const int index = channel->index();
    if(index == kNew || index == kDeleted)
    {
        int fd = channel->fd();
        if (index == kNew)
        {
            assert(_channels.find(fd) == _channels.end());
            _channels[fd] = channel;
        }
        else
        {
            assert(_channels.find(fd) != _channels.end());
            assert(_channels[fd] == channel);
            if(channel->isNoneEvent())
            {
                return;
            }
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else
    {
        int fd = channel->fd();
        (void)fd;
        assert(_channels.find(fd) != _channels.end());
        assert(_channels[fd] == channel);
        assert(index == kAdded);
        if (channel->isNoneEvent())
        {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        }else
        {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPoller::removeChannel(Channel* channel)
{
    assertInLoopThread();
    int fd = channel->fd();
    LOG_TRACE << "fd = " << fd;
    assert(_channels.find(fd) != _channels.end());
    assert(_channels[fd] == channel);
    assert(channel->isNoneEvent());
    int index = channel->index();
    assert(index == kAdded || index == kDeleted);
    size_t n = _channels.erase(fd);
    (void)n;
    assert(n == 1);
    if (index == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

void EPoller::update(int operation, Channel* channel)
{
    struct epoll_event event;
    bzero(&event, sizeof event);
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    if (epoll_ctl(_epollfd, operation, fd, &event) < 0)
    {
        if (operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR << "epoll_ctl op=" << operation << " fd=" << fd;
        }
        else
        {
            LOG_FATAL << "epoll_ctl op=" << operation << " fd=" << fd;
        }
    }
}

