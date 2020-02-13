#include "OnWebPage.h"
#include <string.h>
#include <iostream>
#include <sstream>


OnWebPage::OnWebPage(const string& title,const string& url,const string& contnet)
: _title(title)
, _url(url)
, _content(contnet)
{}

string OnWebPage::summary (const vector<string>& queryWords)
{
    string summary,tmp;
    char buf[200];
    bzero(buf,200);
    stringstream ss(_content);
    getline(ss,tmp);
    while(ss.read(buf,150))
    {
        tmp = string(buf);
        bzero(buf,200);
        for(auto& s: queryWords)
        {
            if(tmp.find(s) != string::npos)
            {
                tmp = "..." + tmp +"...";
                if(summary.size())
                    summary = summary + "\n" + tmp;
                else
                    summary += tmp;
                break;
            }
        }
    }
    return summary;
}



