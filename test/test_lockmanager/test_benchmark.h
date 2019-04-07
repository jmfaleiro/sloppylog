#pragma once

#include <locking_database.h>
#include <gtest/gtest.h>

class BenchmarkTest : public ::testing::Test {
protected:
    unique_ptr<LockingDatabase>     _lck_db;
    int                             _num_records;

    BenchmarkTest();
    ~BenchmarkTest();
    void SetUp();
    void TearDown();
};
