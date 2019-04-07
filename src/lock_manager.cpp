#include <lock_manager.h>

LockManager::LockManager()
{
}

LockManager::~LockManager()
{
}

void LockManager::Lock(DBKey key, shared_ptr<LockRequest> rq)
{
  assert(rq->acquired.load() == false && 
         (rq->mode == READ || rq->mode == WRITE));

  LockHashTable::accessor ac;
  _lock_table.insert(ac, key);
  ac->second.push_front(rq);
  rq->cursor = ac->second.begin();
  rq->rev_cursor = ac->second.rbegin();

  list<shared_ptr<LockRequest>>::const_iterator it = rq->cursor;
  assert(*it == rq);
  ++it;

  if (rq->mode == READ) {
    if (it != ac->second.end() && 
        ((*it)->mode == WRITE || (*it)->acquired == false)) {
      rq->acquired = false;
    }
    else { 
      rq->acquired = true;
    } 
  } else { 
    rq->acquired = (it == ac->second.end());
  }
  extract_deps(key, ac->second);
}

/* Extracts W-W and W-R dependencies */
void LockManager::extract_deps(DBKey key, list<shared_ptr<LockRequest>> sec)
{
  auto fwd = sec.begin();
  auto it = fwd;
  fwd++;

  while (fwd != sec.end()) {
    if ((*fwd)->mode == WRITE) {
      auto write_tx = (*fwd)->tx;
      auto curr_tx = (*it)->tx;
      curr_tx->_commit_deps.insert(write_tx);
      break;
    }
    fwd++;
  }
}

void LockManager::Unlock(DBKey key, shared_ptr<LockRequest> rq)
{
  assert(rq->acquired == true);
  assert(rq->mode == READ || rq->mode == WRITE);

  LockHashTable::accessor ac;

  _lock_table.insert(ac, key);
  auto it_fwd = rq->cursor;
  auto it_bwd = rq->rev_cursor;
  it_fwd++;
  it_bwd++;

  /* Pass the lock */
  if (rq->mode == READ) {
    if (it_fwd == ac->second.end() && 
        (it_bwd != ac->second.rend() && (*it_bwd)->acquired == false)) { 
      assert((*it_bwd)->mode == WRITE); 
      (*it_bwd)->acquired = true;
    }
  } else {
      assert(it_fwd == ac->second.end()); 
      if (it_bwd != ac->second.rend()) {
        (*it_bwd)->acquired = true;
        while (it_bwd != ac->second.rend() && (*it_bwd)->mode == READ) {
          (*it_bwd)->acquired = true;
          it_bwd++;
        }
      }
  }

  /* Splice out from the list */
  ac->second.erase(rq->cursor);
  rq->acquired = false;
}

void LockManager::Reset()
{
  _lock_table.clear();
}
