#ifndef __CACHE_H__
#define __CACHE_H__
#include <unordered_map>
#include <list>
#include <string>
#include <iostream>

using namespace std;

struct CacheNode
{
    string _queryWord;
    string _reuslt;
    CacheNode(const string &key, const string & value)
    : _queryWord(key)
    , _reuslt(value)
    {}
};

class Cache
{
public:
    Cache(size_t capacity);
    Cache(const Cache & rhs);
    void readFromFile(const string & filename);
    void writeToFile(const string & filename);
    void addElement(const string &key, const string & value);
    string get(const string & queryWord);
    void print(){
        for(auto & s:_cacheList)
            cout << s._queryWord << " ";
    }
private:
    size_t _capacity;
    list<CacheNode> _cacheList;
    unordered_map<string,list<CacheNode>::iterator> _hashMap;
};

#endif
