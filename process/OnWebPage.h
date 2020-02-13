#ifndef __ONWEBPAGE_H__
#define __ONWEBPAGE_H__
#include <string>
#include <vector>

using namespace std;


class OnWebPage
{
public:
    OnWebPage(const string& title,const string& url,const string& content);
    string summary (const vector<string>& queryWords);
    string getTitle(){
        return _title;
    }
    string getUrl(){
        return _url;
    }
    string getcontent(){
        return _content;
    }
private:
    string _title;
    string _url;
    string _content;
};

#endif
