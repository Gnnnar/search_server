#pragma once
#include <memory>
#include <unordered_set>
#include <queue>

struct Entry;
class TcpConnection;

typedef std::weak_ptr<TcpConnection> WeakTcpConnectionPtr;
typedef std::shared_ptr<Entry> EntryPtr;
typedef std::weak_ptr<Entry> WeakEntryPtr;
typedef std::unordered_set<EntryPtr> Bucket;
typedef std::queue<Bucket> WeakConnectionList;
