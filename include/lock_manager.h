#pragma once

#include "tbb/concurrent_hash_map.h"
#include <list>

using namespace std;

typedef enum {
  READ = 0,
  WRITE = 1
} LockMode;

struct LockRequest {
  void                                    *tx;
  LockMode                                mode;
  atomic<bool>                            acquired;
  list<LockRequest*>::reverse_iterator    rev_cursor;
  list<LockRequest*>::iterator            cursor;

  LockRequest() : acquired(false)
  {
  }
};

class LockManager {

private:
  tbb::concurrent_hash_map<string, list<LockRequest*>>   _lock_table;  

public:
  LockManager(); 
  ~LockManager();

  void Lock(string key, LockRequest *rq);
  void Unlock(string key, LockRequest *rq);
  void Reset();
};
