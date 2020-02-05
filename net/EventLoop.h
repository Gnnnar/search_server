#pragma once
#include <vector>
#include <memory>
#include "../base/Thread.h"
#include "../base/Timestamp.h"
#include "../base/Mutex.h"
#include "TimerQueue.h"


class Channel;
class Poller;

class EventLoop : noncopyable
{
public:
    typedef std::function<void()> Functor;
    typedef std::function<void()> TimerCallback;
    EventLoop();
    ~EventLoop();

    void loop();

    void quit();
    
    Timestamp pollReturnTime() const {  return _pollReturnTime; }

    void runInLoop(const Functor &cb);

    void queueInLoop(const Functor& cb);

    void runAt(const Timestamp& time, const TimerCallback& cb);
    void runAfter(double delay, const TimerCallback& cb);
    void runEvery(double interval, const TimerCallback& cb);

    void wakeup();

    void updateChannel(Channel* channel);

    void assertInLoopThread()
    {
        if(_thead_id != CurrentThread::tid())
            abortNotInLoopThread();
    }
    
    bool isInLoopThread() const {  return _thead_id == CurrentThread::tid(); }
private:
    typedef std::vector<Channel*> ChannelList;
    void abortNotInLoopThread();
    void handleRead(); 
    void doPendingFunctors();

    bool _looping;
    bool _quit;
    bool _callingPendingFunctors;
    Timestamp _pollReturnTime;
    pid_t _thead_id;
    std::unique_ptr<Poller> _poller;
    std::unique_ptr<TimerQueue> _timerQueue;
    int _wakeupFd;
    std::unique_ptr<Channel> _wakeupChannel;
    ChannelList _activeChannels;
    MutexLock _mutex;
    std::vector<Functor> _pendingFunctors;
};

