#include "MyCache.h"

void MyCache::start()
{
    //readFromFile("../data/cache");
    _running = true;
    _thread.start();
}

void MyCache::write(CacheNode tmp)
{
    MutexLockGuard lock(_write);
    _list.push_back(tmp);
    if(_list.size() > _bar)
    {
        _cond.notify();
    }
}

std::string MyCache::read(std::string word)
{
    string ret;
    {
        MutexLockGuard lock(_read);
        ret = _current->get(word);
    }
    if(ret == "-1")
        return ret;
    write(CacheNode(word,ret));
    return ret;
}

void MyCache::threadFuc()
{
    std::vector<CacheNode> _docker;
    while(_running)
    {
        {
            MutexLockGuard lock(_write);
            _cond.waitForSeconds(_time);
            cout << "wate::" << endl;
            _list.swap(_docker);
        }

        for(auto & node : _docker)
        {
            _clone->addElement(node._queryWord,node._reuslt);
        }

        writeToFile("../data/cache");

        {
            MutexLockGuard lock(_read);
            _current.swap(_clone);
        }

        for(auto & node : _docker)
        {
            _clone->addElement(node._queryWord,node._reuslt);
        }
        
        _docker.clear();
    }
}

void MyCache::readFromFile(const string & filename)
{
    _current->readFromFile(filename);
    _clone->readFromFile(filename);
}

void MyCache::writeToFile(const string & filename)
{
    _clone->writeToFile(filename);
}
