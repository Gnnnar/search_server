#pragma once
#include <map>
#include <vector>
#include <poll.h>
#include "../base/Timestamp.h"
#include "../base/noncopyable.h"
#include "EventLoop.h"

class Channel;

class Poller : noncopyable
{
public:
    typedef std::vector<Channel*> ChannelList;
    Poller(EventLoop* loop);
    ~Poller();

    Timestamp poll(int timeoutMs, ChannelList* activeChannels);

    void updateChannel(Channel* channel);

    void assertInLoopThread() { _ownerLoop->assertInLoopThread();  }

private:
    void fillActiveChannels(int numEvents,ChannelList* activeChannels) const; 
    typedef std::vector<struct pollfd> PollFdList;
    typedef std::map<int, Channel*> ChannelMap;
    EventLoop* _ownerLoop;
    PollFdList  _pollfds;
    ChannelMap _channels;
};

