#include <test_benchmark.h>
#include <test_locking_db.h>
#include <simple_table.h>
#include <unordered_set>
#include <thread>

BenchmarkTest::BenchmarkTest()
{
}

BenchmarkTest::~BenchmarkTest()
{
}

void BenchmarkTest::SetUp()
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

void BenchmarkTest::TearDown()
{
}

TEST_F(BenchmarkTest, SmokeTest)
{
  DBKey key = { "test", "0" };

  auto tx = _lck_db->start_transaction();
  for (auto i = 0; i < _num_records; ++i) {
    key.record = to_string(i);
    auto rec = static_pointer_cast<TestRecord>(_lck_db->read_ref(key, tx));
    ASSERT_EQ(rec->get(), 0);
  }
  _lck_db->commit_transaction_deps(tx);
}

//TEST_F(BenchmarkTest, SmokeTest10x)
//{
//  DBKey key = { "test", "0" };
//  printf("key"); 
//  auto tx = _lck_db->start_transaction();
//  printf("hi");
//  for (auto i = 0; i < 10*_num_records; ++i) {
//    key.record = to_string(i);
//    auto rec = static_pointer_cast<TestRecord>(_lck_db->read_ref(key, tx));
//    ASSERT_EQ(rec->get(), 0);
//  }
//  _lck_db->commit_transaction_deps(tx);
//
//}
//TEST_F(BenchmarkTest, MultiTXTest)
//{
//  DBKey key = { "test", "0" };
//
//  for (auto i = 0; i < _num_records; ++i) {
//    auto tx = _lck_db->start_transaction();
//    key.record = to_string(i);
//    auto rec = static_pointer_cast<TestRecord>(_lck_db->read_ref(key, tx));
//    ASSERT_EQ(rec->get(), 0);
//    _lck_db->commit_transaction_deps(tx);
//    
//  }
//  
//}
