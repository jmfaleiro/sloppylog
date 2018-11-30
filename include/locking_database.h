#pragma once

#include <database.h>
#include <lock_manager.h>
#include <utility>

class LockingTransaction : public Transaction {
friend class LockingDatabase;

private:
  std::unordered_map<DBKey, shared_ptr<LockRequest>> _locks;

};

class LockingDatabase : public Database {
private:
  unique_ptr<LockManager>       _lck_mngr;

  void wait_until_acquired(shared_ptr<LockRequest> lock_rq, shared_ptr<Transaction> tx);

  shared_ptr<Record>
  get_ref(DBKey key, LockMode mode, shared_ptr<LockingTransaction> tx);

public:
  LockingDatabase(unordered_map<std::string, shared_ptr<Table>> init);

  virtual shared_ptr<Record>    read_ref(DBKey key, shared_ptr<Transaction> tx);
  virtual shared_ptr<Record>    write_ref(DBKey key, shared_ptr<Transaction> tx);

  shared_ptr<Transaction>       start_transaction(); 
  bool                          commit_transaction(shared_ptr<Transaction> tx);
};

