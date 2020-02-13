#ifndef __WORDQUERY_H__
#define __WORDQUERY_H__
#include "MyCache.h"
#include "OnWebPage.h"
#include "Configuration.h"
#include <unordered_map>
#include "WordSegmentation.h"

class WordQuery
{
public:
    WordQuery();
    string doQuery(const string & str);
private:
    void loadLibrary();
    vector<double> getBaseW(const map<string,int>& frq,const vector<string>& words);
    string returnNoAnswer();
    vector<int> getDocs(const vector<string>& words);
    string createJson(vector<int> & docIdVec,
                         const vector<string> & queryWords);

private:
    WordSegmentation _jieba;
    MyCache _cache;
    map<string,string>& _conf;
    vector<OnWebPage> _pageLib;
    unordered_map<string, map<int, double> > _invertIndexTable;
};

#endif
