#include <locking_database.h>
#include <lock_manager.h>

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
  // TODO: right placement?
  extract_deps(key);

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

/* Extracts W-W and W-R dependencies */
void LockingDatabase::extract_deps(DBKey key)
{
  LockManager::LockHashTable::accessor ac;
  _lck_mngr->_lock_table.insert(ac, key);
  auto fwd = ac->second.begin();
  auto it = fwd;
  fwd++;

  while (fwd != ac->second.end()) {
    if ((*fwd)->mode == WRITE) {
      while (it != fwd) {
        auto write_tx = (Transaction *) (*fwd).get()->tx;
        auto write_tx_shared = std::make_shared<Transaction>();
        write_tx_shared->_commit_deps = write_tx->_commit_deps;
        auto other_tx = (Transaction *) (*it).get()->tx;
        other_tx->_commit_deps.insert(write_tx_shared);
        it++;
      }
      break;
    }
    fwd++;
  }
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
