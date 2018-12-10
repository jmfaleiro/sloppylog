#include <test_locking_db.h>
#include <simple_table.h>

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

LockingDatabaseTest::~LockingDatabaseTest()
{
}

void LockingDatabaseTest::SetUp()
{
}

void LockingDatabaseTest::TearDown()
{
}

TEST_F(LockingDatabaseTest, SimpleTest)
{
  /*
  DBKey key = { "test", "0" };

  auto tx = _lck_db->start_transaction();
  auto rec = static_pointer_cast<TestRecord>(_lck_db->read_ref(key, tx));
  ASSERT_EQ(rec->get(), 0);
  _lck_db->commit_transaction(tx);
  */
}

