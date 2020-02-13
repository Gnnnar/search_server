#ifndef __PAGELIBPREPROCESSOR_H__
#define __PAGELIBPREPROCESSOR_H__

#include <vector>
#include <unordered_map>
#include <map>
#include <set>

using namespace std;

class WebPage;


class PageLibPreprocessor
{
public:
    PageLibPreprocessor(const map<string,string>& confMap);
    void doProcess();
private:
    void buildWeblib();
    void cutRedundantPages();
    void buildInvertIndexTable();
    void storeOnDisk();
    vector<string> readDict();
private:
    vector<WebPage> _pageLib;
    unordered_map<int, pair<int, int> > _offsetLib;
    unordered_map<string, map<int, double> > _invertIndexTable;
    map<string,string> _confMap;
};

#endif
