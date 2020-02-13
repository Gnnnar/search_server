#ifndef _WD_WORD_SEGMENTATION_H_
#define _WD_WORD_SEGMENTATION_H_

#include "Configuration.h"
#include "../lib/cppjieba/Jieba.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <map>

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::map;


class WordSegmentation
{
public:
	WordSegmentation()
    : _jieba("../lib/cppjieba/dict/jieba.dict.utf8",
                "../lib/cppjieba/dict/hmm_model.utf8",
                "../lib/cppjieba/dict/user.dict.utf8",
                "../lib/cppjieba/dict/idf.utf8",
                "../lib/cppjieba/dict/stop_words.utf8")
	{
		cout << "cppjieba init!" << endl;
	}

	map<string,int> operator()(string text)
	{
		vector<string> words;
        map<string,int> tmp;
		_jieba.Cut(text.c_str(), words, true);//cutAll 
        auto stop = Configuration::getInstance()->getStopWordList();
        for(auto &word : words)
        {
            if(stop.count(word) == 0)
            {
                tmp[word]++;
            }
        }
        return tmp;
	}

private:
	cppjieba::Jieba _jieba;
};



#endif
