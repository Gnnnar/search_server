#include "Server.h"
#include "../net/EventLoop.h"

void Server::onConnection(const TcpConnectionPtr & conn)
{
    if(conn->connected())
    {
        printf("onConnection(): tid=%d new connection [%s] from %s\n",
               CurrentThread::tid(),
               conn->name().c_str(),
               conn->peerAddress().toHostPort().c_str());
    }
    else
    {
        printf("onConnection(): tid=%d connection [%s] is down\n",
               CurrentThread::tid(),
               conn->name().c_str());
    }
}

void Server::onMessage(const TcpConnectionPtr & conn,Buffer* buf,Timestamp tm)
{
    cout << "onMessage...." << endl;
    const char *eof = buf->findEOL();
    if(eof == NULL)
    {
        cout << "error ,data dont full" << endl;
        return;
    }
    string msg = buf->func(eof);
    if(msg.back() == '\n')
        msg.back() = 0;
    cout << ">> receive msg from client: " << msg << endl << tm.toString() <<endl << endl;
    _threadpool.run(std::bind(&Server::doTaskThread,this,conn,msg));
}
/*
   void EchoServer::doTaskThread(const TcpConnectionPtr& conn,const string& msg)
   {
   string ret = _dict.doQuery(msg);
   int lenth = ret.size();
   ret = to_string(lenth) + "\n" + ret;
   conn->sendInLoop(ret);
   }
   */
void Server::doTaskThread(const TcpConnectionPtr & conn, const string & msg)
{
    string ret = _dict.doQuery(msg);

    int sz = ret.size();
    printf("result's size:%d\n",sz); 
    //printf("%s\n\n", ret.c_str());
    //string slength((const char *)&sz, 4);

    string message(to_string(sz));
    message.append("\n").append(ret);
    //printf("message's size:%d\n",message.size()); 
    //conn->sendInLoop(slength);

    conn->send(message);
}

int main()
{
    Configuration::getInstance("../conf/myconf.conf");
    map<string,string> conf = Configuration::getInstance()->getConfigMap();
    EventLoop loop;
    InetAddress dress(conf["ip"],stoi(conf["port"]));
    Server server(stoi(conf["threadNum"]),
                         stoi(conf["queSize"]),
                         &loop,
                         dress);
    server.setIoThreadNum(2);
    server.start();
    loop.loop();
}

