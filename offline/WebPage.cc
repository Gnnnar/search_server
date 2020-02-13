#include "WebPage.h"
#include "Configuration.h"
#include <vector>
#include <sstream>
#include <cmath>
using namespace simhash;

Simhasher WebPage::simhasher = Simhasher("../lib/cppjieba/dict/jieba.dict.utf8"
                    , "../lib/cppjieba/dict/hmm_model.utf8", 
                    "../lib/cppjieba/dict/idf.utf8",
                    "../lib/cppjieba/dict/stop_words.utf8");

cppjieba::Jieba WebPage::jieba = cppjieba::Jieba("../lib/cppjieba/dict/jieba.dict.utf8",
                      "../lib/cppjieba/dict/hmm_model.utf8",
                      "../lib/cppjieba/dict/user.dict.utf8",
                      "../lib/cppjieba/dict/idf.utf8",
                      "../lib/cppjieba/dict/stop_words.utf8");

WebPage::WebPage(const string& title,const string& url,const string& content)
    : _docTitle(title)
    , _docUrl(url)
      , _docContent(content)
{

}

string WebPage::getDoct(int docId)
{
    string tmp = "<doc>";
    tmp = tmp + "\n\t" + "<docid>" + to_string(docId) + "<docid>"
        + "\n\t" + "<url>" + _docUrl + "<url>"
        + "\n\t" + "<title>" + _docTitle + "<title>"
        + "\n\t" + "<content>" + _docContent + "<content>"
        + "\n" + "<doc>" + "\n";
    return tmp;
}

void WebPage::buildU64()
{
    size_t topN = 5;
    vector<pair<string ,double> > res;
    simhasher.extract(_docContent, res, topN);
    simhasher.make(_docContent, topN, _u64);
    //for(auto s :res)
    //cout << "关键词序列是: " << s.first <<":" << s.second << endl;
    // cout<< "simhash值是: " << _u64<<endl;
}

void WebPage::buildWordMap()
{
    auto stopWord = Configuration::getInstance()->getStopWordList();
    using namespace cppjieba;
    stringstream ss(_docContent);
    if(!ss){
        perror("fopen");
        return;
    }
    string s;
    while(ss >> s)
    {
        vector<string> words;
        jieba.Cut(s,words,true);
        for(auto & word:words)
        {
            if(stopWord.count(word) == 0)
                _wordsMap[word]++;
        }
    }
    //    for(auto &s : _wordsMap)
    //  {
    //        cout << s.first << ": " << s.second << endl;
    //    }
    //    cout << endl << endl;
}

void WebPage::getIndexTable(unordered_map<string,map<int,double>> &table,int idx)
{
    for(auto & word : _wordsMap)
    {
        table[word.first].insert(make_pair(idx,0.0));
    }
}

void WebPage::compute(unordered_map<string,map<int,double>> &table,int idx,int sum)
{
    for(auto & word : _wordsMap)
    {
        int tf = word.second;
        double df = table[word.first].size();
        double w = tf * (log(sum/(df+1)));
        _wordW[word.first] = w;
    }
    double sumW = 0;
    for(auto & word : _wordW)
    {
        sumW += word.second * word.second;
    }
    _avgW = sqrt(sumW);
    for(auto & word : _wordW)
    {
        double w = word.second / _avgW;
        auto it = table[word.first].find(idx);
        if(it != table[word.first].end())
        {
            it->second = w;
        }
    }
}


bool WebPage::operator == (const WebPage& rhs)
{
    return Simhasher::isEqual(_u64, rhs._u64);
}

bool WebPage::operator < (const WebPage& rhs)
{
    return _u64 < rhs._u64;
}
