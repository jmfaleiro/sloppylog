#pragma once

#include <database.h>
#include "tbb/concurrent_hash_map.h"
#include <lock_manager.h>

class SimpleTable : public Table {
private:
  typedef tbb::concurrent_hash_map<DBKey, shared_ptr<Record>, DBKey>  SimpleHashTable;
  SimpleHashTable       _hash_tbl;

public:
  SimpleTable();
  shared_ptr<Record>    get_ref(DBKey key);  
  bool                  insert(DBKey key, shared_ptr<Record> record);
  bool                  remove(DBKey key);
};
