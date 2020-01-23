#include "Logging.h"
#include "Thread.h"
#include <string>
#include <unistd.h>
#include <vector>
#include <memory>
#include <iostream>
using namespace std;

void threadFunc()
{
    for (int i = 0; i < 100000; ++i)
    {
        errno = 5;
        LOG_DEBUG << i;
        LOG_INFO << i;
        LOG_ERROR << i;
    }
}

void type_test()
{
    cout << "----------type test-----------" << endl;
    LOG_INFO << 0;
    LOG_DEBUG << 1234567890123;
    LOG_WARN << 1.0f;
    LOG_ERROR << 3.1415926;
    LOG_TRACE << (short) 1;
    LOG_DEBUG << (long long) 1;
    LOG_DEBUG << (unsigned int) 1;
    LOG_DEBUG << (unsigned long) 1;
    LOG_INFO << (double) 1.6555556;
    LOG_INFO << (unsigned long long) 1;
    LOG_INFO << 'c';
    LOG_INFO << "abcdefg";
    LOG_INFO << string("This is a string");
}

void stressing_single_thread()
{
    cout << "----------stressing test single thread-----------" << endl;
    for (int i = 0; i < 100000; ++i)
    {
        LOG_DEBUG << i;
    }
}

void stressing_multi_threads(int threadNum = 4)
{
    cout << "----------stressing test multi thread-----------" << endl;
    vector<shared_ptr<Thread>> vsp;
    for (int i = 0; i < threadNum; ++i)
    {
        shared_ptr<Thread> tmp(new Thread(threadFunc, "testFunc"));
        vsp.push_back(tmp);

    }
    for (int i = 0; i < threadNum; ++i)
    {
        vsp[i]->start();
    }
}

void other()
{
    cout << "----------other test-----------" << endl;
    LOG_INFO << "fddsa" << 'c' << 0 << 3.666 << string("This is a string");
}

int main()
{
    type_test();
    sleep(3);

    stressing_single_thread();
    sleep(3);

    other();
    sleep(3);

    stressing_multi_threads();
    sleep(3);
    return 0;
}
