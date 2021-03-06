#include <locking_database.h>

using namespace std;

LockingDatabase::LockingDatabase(unordered_map<std::string, shared_ptr<Table>> init)
  : Database(init)
{
  _lck_mngr = unique_ptr<LockManager>(new LockManager()); 
}

void LockingDatabase::wait_until_acquired(shared_ptr<LockRequest> lock_rq, 
                                          shared_ptr<Transaction> tx)
{
  while (lock_rq->acquired.load() == false)
    ;
}

shared_ptr<Record>
LockingDatabase::get_ref(DBKey key, LockMode mode, 
                         shared_ptr<LockingTransaction> tx)
{
  assert(mode == READ || mode == WRITE);
  auto lock_rq = shared_ptr<LockRequest>(new LockRequest());
  lock_rq->mode = mode;
  
  _lck_mngr->Lock(key, lock_rq);

  /* XXX Not sure if we need to support upgrading locks. */
  assert(tx->_locks.find(key) == tx->_locks.end());
  tx->_locks[key] = lock_rq;

  wait_until_acquired(lock_rq, tx);
  assert(lock_rq->acquired.load() == true);

  return _tbl_map[key.table]->get_ref(key);
}

shared_ptr<Record>
LockingDatabase::read_ref(DBKey key, shared_ptr<Transaction> tx)
{
  return get_ref(key, READ, dynamic_pointer_cast<LockingTransaction>(tx));
}

shared_ptr<Record>
LockingDatabase::write_ref(DBKey key, shared_ptr<Transaction> tx)
{
  return get_ref(key, WRITE, dynamic_pointer_cast<LockingTransaction>(tx));
}

shared_ptr<Transaction> LockingDatabase::start_transaction() 
{
  auto lck_tx = shared_ptr<LockingTransaction>(new LockingTransaction());
  return static_pointer_cast<Transaction>(lck_tx);
}

/* Release all locks and commit. */
bool LockingDatabase::commit_transaction(shared_ptr<Transaction> tx)
{
  auto lock_tx = std::dynamic_pointer_cast<LockingTransaction>(tx);
  for (auto it = lock_tx->_locks.begin(); it != lock_tx->_locks.end(); ++it) {
    _lck_mngr->Unlock(it->first, it->second); 
  } 

  return true;
}
