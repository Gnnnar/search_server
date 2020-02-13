#pragma once
#include <functional>
#include  <memory>
#include <unordered_set>
#include "../base/Timestamp.h"

class TcpConnection;
class Buffer;
struct Entry;

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

typedef std::function<void()> TimerCallback;
typedef std::function<void (const TcpConnectionPtr&)> ConnectionCallback;
typedef std::function<void (const TcpConnectionPtr&,
                              Buffer* buf,
                              Timestamp tm)> MessageCallback;
typedef std::function<void (const TcpConnectionPtr&)> CloseCallback;

