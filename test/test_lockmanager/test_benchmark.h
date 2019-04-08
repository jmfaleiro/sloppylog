#pragma once

#include <locking_database.h>
#include <gtest/gtest.h>

typedef enum {
  UNIFORM=0,
  ZIPFIAN=1
} Workload;

typedef enum {
  NAIVE=0,
  DEPS=1
} Commit;

class BenchmarkTest : public ::testing::Test {
protected:
    unique_ptr<LockingDatabase>     _lck_db;
    int                             _num_records;
    int                             _num_txs;
    int                             _num_threads;
    BenchmarkTest();
    ~BenchmarkTest();
    void SetUp();
    void TearDown();
    void RunWorkload(Workload workload, Commit commit);
};
