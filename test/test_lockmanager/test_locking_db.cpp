#include <test_locking_db.h>
#include <simple_table.h>
#include <unordered_set>
#include <thread>

TestRecord::TestRecord()
{
  _field_offsets = nullptr; 
  _data = new char[sizeof(uint64_t)];
}

TestRecord::~TestRecord()
{
  delete(_data);
}

uint64_t TestRecord::get()
{
  auto v = (uint64_t*)_data;
  return *v;
}

void TestRecord::set(uint64_t val)
{
  auto v = (uint64_t*)_data;
  *v = val;
}

void TestRecord::reset()
{
  this->set(0); 
}

LockingDatabaseTest::LockingDatabaseTest() 
{
}

LockingDatabaseTest::~LockingDatabaseTest()
{
}

void LockingDatabaseTest::SetUp()
{
  unordered_map<string, shared_ptr<Table>> test_tables; 
  test_tables["test"] = make_shared<SimpleTable>();
  _lck_db = make_unique<LockingDatabase>(test_tables);
  _num_records = 1000;

  for (auto i = 0; i < _num_records; ++i) {
    DBKey key;
    key.table = "test";
    key.record = std::to_string(i);

    shared_ptr<TestRecord> record = make_shared<TestRecord>();
    record->set(0);

    bool ret = test_tables["test"]->insert(key, record);
    assert(ret == true);

    auto get_record = test_tables["test"]->get_ref(key);
    assert(get_record != nullptr);
    assert(record == get_record);
  }
}

void LockingDatabaseTest::TearDown()
{
}

TEST_F(LockingDatabaseTest, SimpleTest)
{
  DBKey key = { "test", "0" };

  auto tx = _lck_db->start_transaction();
  for (auto i = 0; i < _num_records; ++i) {
    key.record = to_string(i);
    auto rec = static_pointer_cast<TestRecord>(_lck_db->read_ref(key, tx));
    ASSERT_EQ(rec->get(), 0);
  }
  _lck_db->commit_transaction_deps(tx);

  tx = _lck_db->start_transaction();
  for (auto i = 0; i < _num_records; ++i) {
    key.record = to_string(i);
    auto rec = static_pointer_cast<TestRecord>(_lck_db->write_ref(key, tx));
    rec->set(1);
  }
  _lck_db->commit_transaction_deps(tx);

  tx = _lck_db->start_transaction();
  for (auto i = 0; i < _num_records; ++i) {
    key.record = to_string(i);
    auto rec = static_pointer_cast<TestRecord>(_lck_db->read_ref(key, tx));
    ASSERT_EQ(rec->get(), 1);
  }
  _lck_db->commit_transaction_deps(tx);
}

static void thread_tx(Database *db, shared_ptr<Transaction> tx, shared_ptr<vector<int>> writeset)
{
  auto num_keys = writeset->size();
  for (auto i = 0; i < num_keys; ++i) {
    DBKey k = { "test", to_string((*writeset)[i]) };
    auto rec = static_pointer_cast<TestRecord>(db->write_ref(k, tx));
    auto prev_val = rec->get();
    rec->set(prev_val + 1);
  }
  db->commit_transaction(tx);
}

static void thread_fn(atomic<int> *tx_counter, 
                      Database *db,
                      int num_txs, 
                      vector<shared_ptr<Transaction>> *txs,
                      vector<shared_ptr<vector<int>>> *writeset)
{
  while (true) {
    auto index = tx_counter->fetch_add(1);
    if (index >= num_txs) {
      return;
    }
    thread_tx(db, (*txs)[index], (*writeset)[index]);
  }
}

//TEST_F(LockingDatabaseTest, TxTest)
//{
//  uint64_t expected_vals[_num_records];
//  for (auto i = 0; i < _num_records; ++i) {
//    expected_vals[i] = 0;
//  }
//
//  int num_txs = 1000;
//  vector<shared_ptr<Transaction>> txs;
//
//  int tx_size = 50;
//  vector<shared_ptr<vector<int>>> write_sets;
//
//  for (auto i = 0; i < num_txs; ++i) {
//    unordered_set<int> seen;
//    ASSERT_EQ(seen.size(), 0);
//
//    while (seen.size() < tx_size) {
//      int val;
//      do {
//        val = rand() % _num_records;
//      } while(seen.find(val) != seen.end());
//      seen.insert(val);
//      expected_vals[val] += 1;
//    }
//
//    ASSERT_EQ(seen.size(), tx_size);
//
//    write_sets.push_back(make_shared<vector<int>>());
//    for (auto it = seen.begin(); it != seen.end(); ++it) {
//      write_sets[i]->push_back(*it);
//    }
//    sort(write_sets[i]->begin(), write_sets[i]->end());
//
//    txs.push_back(_lck_db->start_transaction());
//  }
//
//  int num_threads = 512;
//  thread workers[num_threads];
//  atomic<int> tx_counter = 0;
//
//  for (auto i = 0; i < num_threads; ++i) {
//    workers[i] = std::thread(&thread_fn, &tx_counter, _lck_db.get(), num_txs, &txs, &write_sets);
//  }
//
//  for (auto i = 0; i < num_threads; ++i) {
//    workers[i].join();
//  }
//
//  auto check_tx = _lck_db->start_transaction();
//  for (auto i = 0; i < _num_records; ++i) {
//    DBKey k = { "test", to_string(i) };
//    auto rec = static_pointer_cast<TestRecord>(_lck_db->read_ref(k, check_tx));
//    ASSERT_EQ(rec->get(), expected_vals[i]);
//  }
//  _lck_db->commit_transaction(check_tx);
//}
