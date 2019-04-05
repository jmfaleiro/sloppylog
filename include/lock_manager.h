#pragma once

#include "tbb/concurrent_hash_map.h"
#include <list>
#include <functional>
#include <database.h>

using namespace std;

typedef enum {
  READ = 0,
  WRITE = 1
} LockMode;

struct LockRequest {
  shared_ptr<Transaction>                            tx;
  LockMode                                           mode;
  atomic<bool>                                       acquired;
  list<shared_ptr<LockRequest>>::reverse_iterator    rev_cursor;
  list<shared_ptr<LockRequest>>::iterator            cursor;

  LockRequest() : acquired(false)
  {
  }
};


class LockManager {

private:

public:
  LockManager(); 
  ~LockManager();
  typedef tbb::concurrent_hash_map<DBKey, list<shared_ptr<LockRequest>>, DBKey> LockHashTable;
  LockHashTable _lock_table;
  void Lock(DBKey key, shared_ptr<LockRequest> rq);
  void Unlock(DBKey key, shared_ptr<LockRequest> rq);
  void Reset();
  void extract_deps(DBKey key, list<shared_ptr<LockRequest>> sec);
};
