#include "WebPage.h"
#include "PageLibPreprocessor.h"
#include "tinyxml2.h"
#include <algorithm>
#include <sys/types.h>
#include <cassert>
#include <dirent.h>
#include <regex>
#include <iostream>
#include <fstream>
#include <sstream>

#include "Configuration.h"


using namespace tinyxml2;

PageLibPreprocessor::PageLibPreprocessor(const map<string,string>& confMap)
: _confMap(confMap)
{}

void PageLibPreprocessor::doProcess()
{
    buildWeblib();
    cutRedundantPages();
    buildInvertIndexTable();
    storeOnDisk();
}

void PageLibPreprocessor::buildWeblib()
{
    vector<string> ddir = readDict();
    for(auto &dir:ddir)
    {
        int ret;
        XMLDocument doc;
        ret = doc.LoadFile(dir.c_str());
        assert(ret ==0);
        XMLElement* root = doc.RootElement();
        assert(root != nullptr);
        XMLElement* channel = root->FirstChildElement("channel");
        if(channel == nullptr ){
            continue;
        }
        XMLElement* node = channel->FirstChildElement("item");
        while(node != nullptr)
        {
            regex patten("<[^>]*>");
            string title,url,content;
            XMLElement* pn = node->FirstChildElement("title");
            if(pn->GetText() !=nullptr)
            {
                title = pn->GetText();
                pn = node->FirstChildElement("description");
                if(pn->GetText() !=nullptr)
                {
                    content = title + "\n" + pn->GetText();
                    content = regex_replace(content,patten,"");
                }
            }else
            {
                pn = node->FirstChildElement("description");
                if(pn->GetText() != nullptr)
                {
                    content = pn->GetText();
                    content = regex_replace(content,patten,"");
                    stringstream ss(content);
                    getline(ss,title);
                }
            }
            pn = node->FirstChildElement("link");
            if(pn->GetText() != nullptr){
                url = pn->GetText();
            }
            else{
                node = node->NextSiblingElement("item");
                continue;
            }
            _pageLib.emplace_back(title,url,content);
            node = node->NextSiblingElement("item");
        }
    }
    cout << ">> 提取全部网页库: Ok" << endl;
}

void PageLibPreprocessor::cutRedundantPages()
{
    int i=0;
    cout <<">> 原网页库共:" << _pageLib.size() << "个" << endl;
    for(auto & s:_pageLib)
    {
        s.buildU64();
        cout <<"\r>> 已计算u64值的网页库数量:"<<++i;
        fflush(0);
    }
    sort(_pageLib.begin(),_pageLib.end());
    auto it = unique(_pageLib.begin(),_pageLib.end());
    _pageLib.erase(it,_pageLib.end());
    cout << ">> 删除后网页库数量为:" << _pageLib.size() << "个" << endl;
    cout <<endl<< ">> 删除全部重复网页库: OK" << endl;
}

void PageLibPreprocessor::buildInvertIndexTable()
{
    for(auto & page : _pageLib)
    {
        page.buildWordMap();
    }
    cout << ">> 统计词频: OK" << endl;
    int i=0;
    for(auto & page : _pageLib)
    {
        page.getIndexTable(_invertIndexTable,i++);
    }
    i = 0;
    for(auto & page : _pageLib)
    {
        page.compute(_invertIndexTable,i++,_pageLib.size());
    }
#if 0
    for(auto &a : _invertIndexTable)
    {
        cout << a.first << ": ";
        for(auto &b :a.second)
        {
            cout << b.first << " " << b.second << " // ";
        }
        cout << endl;
    }
# endif
    cout << ">> 建立倒排索引: OK" << endl;
}



void PageLibPreprocessor::storeOnDisk()
{
    ofstream os1(_confMap["pagePath"]);
    ofstream os2(_confMap["offsetPath"]);
    assert(os1 && os2);
    int i = 0;
    for(auto & page: _pageLib)
    {
        string tmp = page.getDoct(i);
        os1 << tmp ;
        int lenth = tmp.size();
        int pos = (int)os1.tellp() - lenth;
        os2 << i << " " << pos << " " << lenth << "\n";
        _offsetLib[i] = make_pair(pos,lenth);
        ++i;
    }
    os1.close();
    os2.close();
    cout << ">> 将全部网页库写入磁盘文件: OK" << endl;
    cout << ">> 将偏移库写入磁盘文件: OK" << endl;
    os1.open(_confMap["indexPath"]);
    assert(os1);
    for(auto &word : _invertIndexTable)
    {
        os1 << word.first << "  ";
        for(auto &pai: word.second)
        {
            os1 << pai.first << " " << pai.second << " ";
        }
        os1 << "\n";
    }
    cout << ">> 将倒排索引写入磁盘文件: OK" << endl;
}





vector<string> PageLibPreprocessor::readDict()
{
    vector<string> tmp;
    string path = _confMap["libPath"];
    struct dirent* pDirInfo;
    DIR* pDir;
    pDir = opendir(path.c_str());
    if(pDir == nullptr){
        perror("opendir");
    }
    while((pDirInfo = readdir(pDir)) != nullptr)
    {
        if(*(pDirInfo->d_name) != '.')
            tmp.push_back(path+string(pDirInfo->d_name));
    }
    closedir(pDir);
    cout << "已读取rss源语料库目录信息:" << endl;
    int i=0;
    for(auto & s : tmp)
        cout <<++i<< s << endl;
    cout << endl;
    return tmp;
}


int main()
{
    Configuration::getInstance("../conf/myconf.conf");
    PageLibPreprocessor produc(Configuration::getInstance()
                               ->getConfigMap());
    produc.doProcess();
    return 0;
}

