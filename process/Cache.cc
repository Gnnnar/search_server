#include "Cache.h"
#include <fstream>


Cache::Cache(size_t capacity)
    : _capacity(capacity)
{}

Cache::Cache(const Cache & rhs)
    : _capacity(rhs._capacity)
{
    for(auto node = rhs._cacheList.rbegin();
        node != rhs._cacheList.rend();node++)
    {
        addElement(node->_queryWord,node->_reuslt);
    }
}

void Cache::addElement(const string &key, const string & value)
{
    auto it = _hashMap.find(key);
    if(it == _hashMap.end())
    {
        if(_cacheList.size() == _capacity)
        {
            string kkey = _cacheList.back()._queryWord;
            _hashMap.erase(kkey);
            _cacheList.pop_back();
        }
        _cacheList.push_front(CacheNode(key,value));
        //_hashMap[key] = _cacheList.begin();
        _hashMap.insert(make_pair(key,_cacheList.begin()));
    }
    else
    {
        //it->second->_reuslt = value;
        _cacheList.splice(_cacheList.begin(),
                          _cacheList,it->second);
        it->second = _cacheList.begin();
    }
}

void Cache::writeToFile(const string & filename)
{
    ofstream os(filename);
    if(!os){
        perror("fopen");
        return;
    }
    for(auto &node : _cacheList)
    {
        if(node._reuslt.back()=='\n'){
            node._reuslt.resize(node._reuslt.size()-1);
        }
        os << node._queryWord << " "
            << node._reuslt << '\n';
    }
    os.close();
}

void Cache::readFromFile(const string & filename)
{
    ifstream is(filename);
    if(!is){
        perror("fopen");
        return;
    }
    string key,value;
    while(is)
    {
        is >> key >> value;
        if(value.back()=='\n'){
            value.resize(value.size()-1);
        }
        _cacheList.push_back(CacheNode(key,value));
        _hashMap[key] = --_cacheList.end();
    }
    is.close();
}

string Cache::get(const string & queryWord)
{
    auto it = _hashMap.find(queryWord);
    if(it == _hashMap.end()){
        return string("-1");
    }else{
        return it->second->_reuslt;
    }
}

#if 0
int main()
{
    Cache cache(4);
    cache.addElement("11","aa");
    cache.addElement("22","aa");
    cache.addElement("33","aa");
    cache.addElement("44","aa");
    cache.print();
    cout << endl;
    Cache ca(cache);
    ca.print();
    cout << endl;
}





int main()
{
    shared_ptr<Cache> pC(new Cache(6));
    pC->addElement("111","aaa");
    pC->addElement("222","bbb");
    pC->addElement("333","ccc");
    pC->addElement("444","ddd");
    pC->addElement("555","ddd");
    pC->addElement("666","ddd");
    pC->print();
    cout <<endl;
    Cache cache(*(pC.get()));
    cache.print();
    //shared_ptr<Cache> pc(new Cache(*pC));
    //pc->print();
    cout <<endl;
    cout <<endl;
    //    cache.writeToFile("test.txt");
    return 0;
}

int main()
{

    Cache cache(3);
    cache.readFromFile("test.txt");
    cout << cache.get("111") << endl;
    cout << cache.get("222") << endl;
    cout << cache.get("333") << endl;
    cout << cache.get("444") << endl;
}

#endif

