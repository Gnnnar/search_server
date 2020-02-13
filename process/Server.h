#ifndef __ECHOSERVER_H__
#define __ECHOSERVER_H__
#include "../net/TcpConnection.h"
#include "../net/TcpServer.h"
#include "../base/ThreadPool.h"
#include "WordQuery.h"
#include <iostream>

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using namespace std;
class Server
{
public:
    Server(size_t threadNum,size_t queSize,EventLoop *loop,const InetAddress& listenAddr)
    : _threadpool()
    , _server(loop,listenAddr)
    , _threadNum(threadNum)
    {
        _threadpool.setMaxQueueSize(queSize);
    }
    void onConnection(const TcpConnectionPtr & conn);
    void onMessage(const TcpConnectionPtr & conn,Buffer* buf,Timestamp tm);
    void doTaskThread(const TcpConnectionPtr& conn,const string& msg);
    void setIoThreadNum(int num) {  _server.setThreadNum(num); }
    void start()
    {
        _threadpool.start(_threadNum);
        _server.setConnectionCallback(std::bind(&Server::onConnection,this,_1));
        _server.setMessageCallback(std::bind(&Server::onMessage,this,_1,_2,_3));
        _server.start();
    }
private:
    WordQuery _dict;
    ThreadPool _threadpool;
    TcpServer _server;
    size_t _threadNum;
};


#endif
