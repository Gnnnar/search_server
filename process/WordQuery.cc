#include "../lib/json/json.h"
#include "WordQuery.h"
//#include "../include/redis.h"
//#include <hiredis/hiredis.h> 

//extern __thread Redis* predis;

struct SimilarityCompare
{
	SimilarityCompare(vector<double> & base)
	: _base(base)
	{}


	bool operator()(const pair<int, vector<double> > & lhs, 
					const pair<int, vector<double> > & rhs)
	{	// 都与基准向量进行计算
		double lhsCrossProduct = 0;
		double rhsCrossProduct = 0;
		double lhsVectorLength = 0;
		double rhsVectorLength = 0;
		for(int index = 0; index != (int)_base.size(); ++index)
		{
			lhsCrossProduct += (lhs.second)[index] * _base[index];
			rhsCrossProduct += (rhs.second)[index] * _base[index];
			lhsVectorLength += pow((lhs.second)[index], 2);
			rhsVectorLength += pow((rhs.second)[index], 2);
		}
		if(lhsCrossProduct / sqrt(lhsVectorLength) < rhsCrossProduct / sqrt(rhsVectorLength))
		{	return false;	}
		else
		{	return true;	}
	}
    vector<double> _base;
};


WordQuery::WordQuery()
    : _conf(Configuration::getInstance()->getConfigMap())
{
    loadLibrary();
}

void WordQuery::loadLibrary()
{
    int docId,beg,lenth;
    string title,url,content;
    char buf[102400];
    ifstream offis(_conf["offsetPath"]);
    ifstream pageis(_conf["pagePath"]);
    assert( offis && pageis );
    while(offis >> docId >> beg >> lenth)
    {   
        pageis.read(buf,lenth);
        string text(buf);
        int bbeg = text.find("<url>") + 5;
        int eend = text.find("<url>",bbeg);
        url = text.substr(bbeg,eend-bbeg);
        bbeg = text.find("<title>") + 7;
        eend = text.find("<title>",bbeg);
        title = text.substr(bbeg,eend-bbeg);
        bbeg = text.find("<content>") + 9;
        eend = text.find("<content>",bbeg);
        content = text.substr(bbeg,eend-bbeg);
        _pageLib.emplace_back(title,url,content);
    }
    offis.close();
    pageis.close();
    ifstream is(_conf["indexPath"]);
    string line;
    while(getline(is,line))
    {
        stringstream ss(line);
        string word;
        ss >> word;
        int docId;
        double w;
        while(ss >> docId >> w)
        {
            _invertIndexTable[word].insert(make_pair(docId,w));
        }
    }
    is.close();
#if 0
    int i=0;
    for(auto page:_pageLib)
        cout << i++ <<endl
            << page.getTitle() << endl
            << page.getUrl() << endl
            << page.getcontent() << endl;
    int i=0;
    for(auto s: _invertIndexTable)
    {
        cout << i++  
            << s.first <<"  " ;
        for(auto b:s.second)
            cout << b.first <<" " << b.second << " ";
        cout << endl;
    }
    auto it = _invertIndexTable["手册"];
    for(auto s: it)
        cout << s.first <<" " << s.second << endl;
#endif
}

string WordQuery::doQuery(const string & str)
{
    if(str.size() == 0)
        return returnNoAnswer();
    string ret;
/*    string ret = predis->get(str);
    if(ret != "-1")
    {
        cout << ">> 缓存命中" << endl;
        return ret;
    }
    cout << ">> 缓存未命中,将进行计算" <<endl;
*/
    map<string,int> frq = _jieba(str);
    vector<string> words;
    vector<pair<int,vector<double>>> resultList;
    if(frq.size() == 0)
    {
        ret =  returnNoAnswer();
    }
    for(auto &s:frq)
    {
        if(_invertIndexTable.find(s.first) == _invertIndexTable.end())
        {
            ret =  returnNoAnswer();
//            predis->set(str,ret);
            return ret;
        }
        words.push_back(s.first);
    }
    vector<double> baseW = getBaseW(frq,words);
    vector<int> docs = getDocs(words);
    if(docs.size() == 0)
    {
        ret =  returnNoAnswer();
//        predis->set(str,ret);
        return ret;
    }
    for(auto docId : docs)
    {
        vector<double> tmp;
        for(auto &word:words)
        {
            auto mmp = _invertIndexTable.find(word)->second;
            tmp.push_back(mmp.find(docId)->second);
        }
        resultList.emplace_back(make_pair(docId,tmp));
    }
    SimilarityCompare compair(baseW);
    stable_sort(resultList.begin(),resultList.end(),compair);
    docs.clear();
    for(auto & s: resultList)
        docs.push_back(s.first);
    ret = createJson(docs,words);
//    predis->set(str,ret);
    return ret;
#if 0
    for(auto s:docs)
        cout << s <<" ";
    cout << endl;
    for(auto s:resultList)
    {
        cout << s.first << ":";
        for(auto b: s.second)
            cout << b <<" ";
        cout << endl;
    }
#endif
}

vector<double> WordQuery::getBaseW(const map<string,int>& frq,const vector<string>& words)
{
    vector<double> wVal;
    double sumW = 0;
    for(auto & word:words)
    {
        double df = _invertIndexTable.find(word)->second.size();
        double tf = frq.find(word)->second;
        double w = tf*log(_pageLib.size() / (df+1));
        wVal.push_back(w);
        sumW += w*w;
    }
    sumW = sqrt(sumW);
    for(auto &val :wVal)
        val /= sumW;
    return wVal;
}

vector<int> WordQuery::getDocs(const vector<string>& words)
{
    vector<int> docs;
    for(auto& pai : _invertIndexTable[words[0]])
        docs.push_back(pai.first);
    sort(docs.begin(),docs.end());
    for(auto it = words.begin()+1;it != words.end();it++)
    {
        vector<int> rhs,tmp;
        for(auto& pai : _invertIndexTable[*it])
            rhs.push_back(pai.first);
        sort(rhs.begin(),rhs.end());
        set_intersection(docs.begin(),docs.end(),
                         rhs.begin(),rhs.end()
                         ,back_inserter(tmp));
        docs.swap(tmp);
    }
    return docs;
}

string WordQuery::createJson(vector<int> & docIdVec, const vector<string> & queryWords)
{
    Json::Value json;
    Json::StyledWriter wrrit;
    int i = 0;
    for(auto docId:docIdVec)
    {
        if(i++ > 50)
            break;
        Json::Value tmp;
        tmp["title"] = _pageLib[docId].getTitle();
        tmp["url"] = _pageLib[docId].getUrl();
        tmp["summary"] = _pageLib[docId].summary(queryWords);
        json["files"].append(tmp);
    }
    return wrrit.write(json);
}
/*
string WordQuery::returnNoAnswer()
{
	Json::Value root;
	Json::Value arr;

	Json::Value elem;
	elem["title"] = "404, not found";
	elem["summary"] = "亲,I cannot find what you want. What a pity!";
	elem["url"] = "";
	arr.append(elem);
	root["files"] = arr;
	Json::StyledWriter writer;
	return writer.write(root);
}*/

string WordQuery::returnNoAnswer()
{
	Json::Value root;
	Json::Value arr;

	Json::Value elem;
	elem["title"] = "404, not found";
	elem["summary"] = "亲,I cannot find what you want. What a pity!";
	elem["url"] = "";
	arr.append(elem);
	root["files"] = arr;
	Json::StyledWriter writer;
	return writer.write(root);
}
#if 0
int main()
{
    Configuration::getInstance("../conf/myconf.conf");
    WordQuery w;
    cout << w.doQuery("安卓") << endl;
    return 0;
}
#endif
