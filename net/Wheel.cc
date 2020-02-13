#include "Wheel.h"
#include "TcpConnection.h"

struct Entry
{
    Entry(const WeakTcpConnectionPtr& weakConn)
    :_weakConn(weakConn)
    {}

     ~Entry()
     {
         TcpConnectionPtr conn = _weakConn.lock();
         if(conn)
         {
             conn->forceClose();
         }
     }
     WeakTcpConnectionPtr _weakConn;
}
;
