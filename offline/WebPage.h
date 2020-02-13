#ifndef __WEBPAGE_H__
#define __WEBPAGE_H__

#include <unordered_map>
#include <map>
#include <string>
#include <set>
#include "../lib/simhash/Simhasher.hpp"
#include "../lib/cppjieba/Jieba.hpp"


using namespace std;


class WebPage
{
public:
    WebPage(const string &title,const string& url,const string& content);
    string getDoct(int docId);
    map<string, int> & getWordsMap();
    bool operator == (const WebPage& rhs);
    bool operator < (const WebPage& rhs);
    void buildU64();
    void buildWordMap();
    void getIndexTable(unordered_map<string,map<int,double>> &table,int idx);
    void compute(unordered_map<string,map<int,double>> &table
                 ,int idx,int sum);
private:
private:
    string _docTitle;
    string _docUrl;
    string _docContent;
    map<string,int> _wordsMap;
    map<string,double> _wordW;
    double _avgW;
    uint64_t _u64 = 0;
    static simhash::Simhasher simhasher;
    static cppjieba::Jieba jieba;
};

#endif
