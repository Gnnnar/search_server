#include <iostream>

#include "Poller.h"
#include "Channel.h"
#include "../base/Logging.h"

Poller::Poller(EventLoop* loop)
: _ownerLoop(loop)
{}

Poller::~Poller()
{}

Timestamp Poller::poll(int timeoutMs, ChannelList* activeChannels)
{
    int numEvent = ::poll(&*_pollfds.begin(),_pollfds.size(),timeoutMs);
    Timestamp now(Timestamp::now());
    if(numEvent > 0)
    {
        LOG_TRACE << numEvent << " events happended";
        fillActiveChannels(numEvent,activeChannels);
    }
    else if(numEvent == 0)
    {
        LOG_TRACE << " nothing happended";
    }
    else
    {
        LOG_ERROR << "Poller::poll()";
    }
    return now;
}

void Poller::fillActiveChannels(int numEvents,ChannelList* activeChannels) const
{
    for(auto pfd = _pollfds.begin();pfd != _pollfds.end() && numEvents >0 ;pfd++)
    {
        if(pfd->revents > 0)
        {
            --numEvents;
            auto ch = _channels.find(pfd->fd);
            assert(ch != _channels.end());
            Channel* channel = ch->second;
            assert(channel->fd() == pfd->fd);
            channel->set_revents(pfd->revents);
            activeChannels->push_back(channel);
        }
    }
}

void Poller::updateChannel(Channel* channel)
{
    assertInLoopThread();
    LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();
    if(channel->index() < 0)
    {
        assert(_channels.find(channel->fd()) == _channels.end());
        struct pollfd pfd;
        pfd.fd = channel->fd();
        pfd.events = channel->events();
        pfd.revents = 0;
        _pollfds.push_back(pfd);
        channel->set_index(_pollfds.size()-1);
        _channels[channel->fd()] = channel;
    }else
    {
        assert(_channels.find(channel->fd()) != _channels.end());
        assert(_channels[channel->fd()] == channel);
        int idx = channel->index();
        assert(0 <= idx && idx < static_cast<int>(_pollfds.size()));
        struct pollfd& pfd = _pollfds[idx];
        assert(pfd.fd == channel->fd() || pfd.fd == -1);
        pfd.events = static_cast<short>(channel->events());
        pfd.revents = 0;
        if(channel->isNoneEvent())
        {
            pfd.fd = -1;
        }
    }
}
