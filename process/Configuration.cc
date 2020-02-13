#include "Configuration.h"
#include <iostream>
#include <fstream>
#include <cassert>
using namespace std;


Configuration* Configuration::_pInstance = nullptr;

Configuration* Configuration::getInstance()
{
    if(_pInstance)
        return _pInstance;
    else
        return nullptr;
}

Configuration* Configuration::getInstance(const string & filePath)
{
    if(!_pInstance)
    {
        _pInstance = new Configuration(filePath);
        atexit(destroy);
    }
    return _pInstance;
}

void Configuration::destroy()
{
    if(_pInstance)
        delete _pInstance;
}

map<string,string>& Configuration::getConfigMap()
{
    return _configMap;
}

Configuration::Configuration(const string & filePath)
: _filePath(filePath)
{
    ifstream is(_filePath);
    assert(is);
    string idx,content;
    while(is)
    {
        is >> idx >> content;
        _configMap[idx] = content;
    }
    is.close();
    is.open(_configMap["stopWordPath"]);
    assert(is);
    while(is >> content)
    {
        _stopWordList.insert(content);
    }
    is.close();
//    cout << "Configuration(const string & filePath)" << endl;
    cout << "已读取配置文件" << endl;
}

Configuration::~Configuration()
{
//    cout << "~Configuration()" << endl;
}

set<string>& Configuration::getStopWordList()
{
    return _stopWordList;
}




#if 0
int main()
{
    Configuration* p = Configuration::getInstance("../conf/myconf.conf");
    for(auto &s: p->getConfigMap())
    {
        cout << s.first << " " << s.second << endl;
    }
    cout << p->getConfigMap()["libPath"] << endl;
    cout << p->getStopWordList().count("a")<< endl;
    return 0;
}
#endif
