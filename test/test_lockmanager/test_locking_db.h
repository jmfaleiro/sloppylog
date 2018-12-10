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
  
  LockingDatabaseTest();
  ~LockingDatabaseTest();

  virtual void SetUp();

  virtual void TearDown();

  unique_ptr<LockingDatabase>       _lck_db; 
  int                               _num_records;
};
