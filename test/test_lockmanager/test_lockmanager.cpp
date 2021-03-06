#include <test_lockmanager.h>
#include <vector>
#include <cstdlib>
#include <thread>
#include <set>

LockManagerTest::LockManagerTest()
{
}

LockManagerTest::~LockManagerTest()
{
}

void LockManagerTest::SetUp()
{
}

void LockManagerTest::TearDown()
{
  _lck_mngr.Reset();  
}

TEST_F(LockManagerTest, WriteWriteTest)
{
  auto rq = std::make_shared<LockRequest>();
  auto rq2 = std::make_shared<LockRequest>();

  rq->mode = WRITE;
  rq2->mode = WRITE;

  DBKey key = { "tbl", "test" };

  _lck_mngr.Lock(key, rq);
  ASSERT_EQ(rq->acquired.load(), true);

  _lck_mngr.Lock(key, rq2); 
  ASSERT_EQ(rq2->acquired.load(), false);

  _lck_mngr.Unlock(key, rq); 
  ASSERT_EQ(rq->acquired.load(), false);
  ASSERT_EQ(rq2->acquired.load(), true);
}

TEST_F(LockManagerTest, ReadWriteTest)
{
  auto rqRead1 = make_shared<LockRequest>();
  auto rqRead2 = make_shared<LockRequest>(); 
  auto rqRead3 = make_shared<LockRequest>();
  auto rqWrite1 = make_shared<LockRequest>();
  auto rqWrite2 = make_shared<LockRequest>();

  rqRead1->mode = READ;
  rqRead2->mode = READ;
  rqRead3->mode = READ;
  rqWrite1->mode = WRITE;
  rqWrite2->mode = WRITE;

  DBKey key = { "tbl", "test" };

  _lck_mngr.Lock(key, rqRead1);
  ASSERT_EQ(rqRead1->acquired.load(), true);

  _lck_mngr.Lock(key, rqWrite1);
  ASSERT_EQ(rqRead1->acquired.load(), true);
  ASSERT_EQ(rqWrite1->acquired.load(), false); 

  _lck_mngr.Lock(key, rqRead2);
  ASSERT_EQ(rqRead1->acquired.load(), true);
  ASSERT_EQ(rqWrite1->acquired.load(), false);
  ASSERT_EQ(rqRead2->acquired.load(), false);

  _lck_mngr.Lock(key, rqRead3);
  ASSERT_EQ(rqRead1->acquired.load(), true);
  ASSERT_EQ(rqWrite1->acquired.load(), false);
  ASSERT_EQ(rqRead2->acquired.load(), false);
  ASSERT_EQ(rqRead3->acquired.load(), false);

  _lck_mngr.Lock(key, rqWrite2);
  ASSERT_EQ(rqRead1->acquired.load(), true);
  ASSERT_EQ(rqWrite1->acquired.load(), false);
  ASSERT_EQ(rqRead2->acquired.load(), false);
  ASSERT_EQ(rqRead3->acquired.load(), false);
  ASSERT_EQ(rqWrite2->acquired.load(), false);

  _lck_mngr.Unlock(key, rqRead1);
  ASSERT_EQ(rqRead1->acquired.load(), false);
  ASSERT_EQ(rqWrite1->acquired.load(), true);
  ASSERT_EQ(rqRead2->acquired.load(), false);
  ASSERT_EQ(rqRead3->acquired.load(), false);
  ASSERT_EQ(rqWrite2->acquired.load(), false);

  _lck_mngr.Unlock(key, rqWrite1);
  ASSERT_EQ(rqRead1->acquired.load(), false);
  ASSERT_EQ(rqWrite1->acquired.load(), false);
  ASSERT_EQ(rqRead2->acquired.load(), true);
  ASSERT_EQ(rqRead3->acquired.load(), true);
  ASSERT_EQ(rqWrite2->acquired.load(), false);

  _lck_mngr.Unlock(key, rqRead3);
  ASSERT_EQ(rqRead1->acquired.load(), false);
  ASSERT_EQ(rqWrite1->acquired.load(), false);
  ASSERT_EQ(rqRead2->acquired.load(), true);
  ASSERT_EQ(rqRead3->acquired.load(), false);
  ASSERT_EQ(rqWrite2->acquired.load(), false);

  _lck_mngr.Unlock(key, rqRead2);
  ASSERT_EQ(rqRead1->acquired.load(), false);
  ASSERT_EQ(rqWrite1->acquired.load(), false);
  ASSERT_EQ(rqRead2->acquired.load(), false);
  ASSERT_EQ(rqRead3->acquired.load(), false);
  ASSERT_EQ(rqWrite2->acquired.load(), true);

  _lck_mngr.Unlock(key, rqWrite2);
  ASSERT_EQ(rqRead1->acquired.load(), false);
  ASSERT_EQ(rqWrite1->acquired.load(), false);
  ASSERT_EQ(rqRead2->acquired.load(), false);
  ASSERT_EQ(rqRead3->acquired.load(), false);
  ASSERT_EQ(rqWrite2->acquired.load(), false);
}

static void acquire_lock(shared_ptr<LockRequest> rq, LockManager *lck_mngr)
{
  DBKey key = { "tbl", "test" };
  lck_mngr->Lock(key, rq);
}

static void test_array(vector<shared_ptr<LockRequest>> &rq_array, std::set<int> &locked,  
                       int num_reqs)
{
  ASSERT_EQ(locked.size(), 0);
  for (int i = 0; i < num_reqs; ++i) {
    if (rq_array[i]->acquired) { 
      locked.insert(i);
    }
  }
}

TEST_F(LockManagerTest, ConcurrentTest)
{
  using namespace std;

  int num_requests = 5000;
  vector<shared_ptr<LockRequest>> rq_array;
  set<int> locked;
  DBKey key = { "tbl", "test" };
  thread thrd_array[num_requests];
  bool prev_write = true;

  for (int i = 0; i < num_requests; ++i) {
    rq_array.push_back(make_shared<LockRequest>());
    rq_array[i]->mode = (rand() % 2 == 0? WRITE : READ);
    rq_array[i]->acquired = false; 
  }

  for (int i = 0; i < num_requests; ++i) {
    thrd_array[i] = std::thread(&acquire_lock, rq_array[i], &_lck_mngr);
  }

  for (int i = 0; i < num_requests; ++i) {
    thrd_array[i].join();
  }

  while (true) {
    set<int> newly_locked;  
    test_array(rq_array, newly_locked, num_requests); 

    if (newly_locked.size() == 0) {
      ASSERT_EQ(locked.size(), num_requests);
      break;
    }
    
    auto it = newly_locked.begin();
    if (rq_array[*it]->mode == READ) {
      ASSERT_EQ(prev_write, true);
      while (it != newly_locked.end()) {
        ASSERT_EQ(rq_array[*it]->acquired.load(), true);
        ASSERT_EQ(rq_array[*it]->mode, READ);
        _lck_mngr.Unlock(key, rq_array[*it]);
        locked.insert(*it);
        ++it;
      }
      prev_write = false;
    } else {
      ASSERT_EQ(rq_array[*it]->acquired.load(), true);
      ASSERT_EQ(newly_locked.size(), 1);
      _lck_mngr.Unlock(key, rq_array[*it]);
      locked.insert(*it);
      ++it;
      ASSERT_EQ(it, newly_locked.end());
      prev_write = true;
    }
  }
}
