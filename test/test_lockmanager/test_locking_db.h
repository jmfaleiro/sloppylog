#pragma once

#include <locking_database.h>
#include <gtest/gtest.h>

class TestRecord : public Record {
public:
  TestRecord();
  ~TestRecord();

  uint64_t get();
  void set(uint64_t val);
  void reset();
};

class LockingDatabaseTest : public ::testing::Test {
protected:
  unique_ptr<LockingDatabase>       _lck_db; 
  int                               _num_records;

//  void thread_tx(shared_ptr<Transaction> tx, shared_ptr<vector<int>> writeset);


  LockingDatabaseTest();
  ~LockingDatabaseTest();
  void SetUp();
  void TearDown();

public:
/*
  void thread_fn(atomic<int> &tx_counter, 
                 int num_txs, 
                 vector<shared_ptr<Transaction>> &txs,
                 shared_ptr<vector<int>> writeset[]);
                 */

};
