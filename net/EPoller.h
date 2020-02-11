#pragma once
#include <map>
#include <vector>
#include "../base/Timestamp.h"
#include "../base/noncopyable.h"
#include "EventLoop.h"

class Channel;

class EPoller
{
public:
    typedef std::vector<Channel*> ChannelList;
    EPoller(EventLoop* loop);
    ~EPoller();

    Timestamp poll(int timeoutMs, ChannelList* activeChannels);

    void updateChannel(Channel* channel);

    void removeChannel(Channel* channel);

    void assertInLoopThread() { _ownerLoop->assertInLoopThread();  }

private:
    typedef std::vector<struct epoll_event> EventList;
    typedef std::map<int, Channel*> ChannelMap;

    static const int kInitEventListSize = 16;

    void fillActiveChannels(int numEvents,
                            ChannelList* activeChannels) const;
    void update(int operation, Channel* channel);

    EventLoop* _ownerLoop;
    int _epollfd;
    EventList _events;
    ChannelMap _channels;
};

