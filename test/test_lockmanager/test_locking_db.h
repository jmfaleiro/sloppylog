#pragma once

#include <locking_database.h>
#include <gtest/gtest.h>

class TestRecord : public Record {
public:
  TestRecord() {
    _field_offsets = nullptr; 
    _data = new char[sizeof(uint64_t)];
  }

  ~TestRecord() {
    delete(_data);
  }

  uint64_t get() {
    auto v = (uint64_t*)_data;
    return *v;
  }

  void set(uint64_t val) {
    auto v = (uint64_t*)_data;
    *v = val;
  }

  void reset() {
    this->set(0); 
  }
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
