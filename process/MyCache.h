#pragma once
#include "Cache.h"
#include "../base/Mutex.h"
#include "../base/Thread.h"
#include "../base/Condition.h"
#include <memory>
#include <vector>


class MyCache : noncopyable
{
public:
    MyCache() 
    : _running(false)
    , _cond(_write)
    , _thread(std::bind(&MyCache::threadFuc,this),"Cache")
    , _current(new Cache(_capacity))
    , _clone(new Cache(_capacity))
    {}

    void start();

    void write(CacheNode tmp);

    std::string read(std::string word);

private:
    void threadFuc();
    void readFromFile(const string & filename);
    void writeToFile(const string & filename);
    
    bool _running;
    MutexLock _read;
    MutexLock _write;
    Condition _cond;
    Thread _thread;
    std::shared_ptr<Cache> _current;
    std::shared_ptr<Cache> _clone;
    std::vector<CacheNode> _list;
    const static int _capacity = 1000;
    const static int _time = 5;
    const static int _bar = 10;
};

