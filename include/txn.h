#pragma once

#include <deque>
#include <lock_manager.h>

class Record;

class Transaction {

using namespace std;

private:
  Database                *_db;     // Table of tables. 
  deque<LockRequest*>     _locks;   // Locks held by the transaction.  
  deque<Record*>          _writes;  // Written out to stable storage.
  
  void Execute() = 0;

};
