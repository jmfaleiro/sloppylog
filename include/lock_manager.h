#pragma once

#include "tbb/concurrent_hash_map.h"
#include <list>
#include <functional> 

using namespace std;

typedef enum {
  READ = 0,
  WRITE = 1
} LockMode;

struct LockRequest {
  void                                               *tx;
  LockMode                                           mode;
  atomic<bool>                                       acquired;
  list<shared_ptr<LockRequest>>::reverse_iterator    rev_cursor;
  list<shared_ptr<LockRequest>>::iterator            cursor;

  LockRequest() : acquired(false)
  {
  }
};

struct DBKey {
  string      table;
  string      record;

  bool operator==(const DBKey &other) const 
  {
    return this->table == other.table && this->record == other.record;
  }

  static size_t hash(const DBKey& k)
  {
    auto table_hash = std::hash<string>{}(k.table);
    auto record_hash = std::hash<string>{}(k.record);
    return (table_hash << 1) ^ record_hash;
  }

  static bool equal(const DBKey& k1, const DBKey& k2)
  {
    return k1.table == k2.table && k1.record == k2.record;
  }
};

template<>
struct hash<DBKey>
{
  std::size_t operator()(const DBKey &k) const
  {
    return DBKey::hash(k);
  }
};

class LockManager {

private:
  typedef tbb::concurrent_hash_map<DBKey, list<shared_ptr<LockRequest>>, DBKey> LockHashTable;
  LockHashTable   _lock_table;  

public:
  LockManager(); 
  ~LockManager();

  void Lock(DBKey key, shared_ptr<LockRequest> rq);
  void Unlock(DBKey key, shared_ptr<LockRequest> rq);
  void Reset();
};
