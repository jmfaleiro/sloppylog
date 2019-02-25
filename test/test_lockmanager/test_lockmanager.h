#pragma once

#include <database.h>
#include <lock_manager.h>
#include <gtest/gtest.h>

class LockManagerTest : public ::testing::Test {

protected:
  
  LockManagerTest();

  virtual ~LockManagerTest();

  virtual void SetUp();

  virtual void TearDown();

  LockManager _lck_mngr; 
};
