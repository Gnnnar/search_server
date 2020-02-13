#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__
#include <string>
#include <map>
#include <pthread.h>
#include <set>
using std::map;
using std::string;
using std::set;

class Configuration
{
public:
    static Configuration* getInstance();
    static Configuration* getInstance(const string & filePath);
    static void destroy();
    map<string,string>& getConfigMap();
    set<string>& getStopWordList();
private:
    Configuration(const string & filePath);
    ~Configuration();
private:
    static Configuration* _pInstance;
    string _filePath;
    map<string,string> _configMap;
    set<string> _stopWordList;
};

#endif
