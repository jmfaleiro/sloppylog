#include <test_benchmark.h>
#include <test_locking_db.h>
#include <simple_table.h>
#include <unordered_set>
#include <thread>
#include <chrono>

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
  /*
  _num_records = 1000;
  _num_txs = 1000;
  _num_threads = 10;
  */  
  _num_records = 1000000;
  _num_txs = 1000000;
  _num_threads = 512;
 
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

static void thread_fn(atomic<int> *tx_counter, LockingDatabase *db, int num_txs, 
                      vector<shared_ptr<Transaction>> *txs,
                      vector<shared_ptr<int>> *writesets, bool use_deps)
{
  while (true) {
    auto index = tx_counter->fetch_add(1);
    if (index >= num_txs) {
      return;
    }
    DBKey k = {"test", to_string(*(*writesets)[index])};
    auto rec = static_pointer_cast<TestRecord>(db->write_ref(k, (*txs)[index]));
    auto prev_val = rec->get();
    rec->set(prev_val + 1);
    if (use_deps == true) {
      db->commit_transaction_deps((*txs)[index]);
    } else {
      db->commit_transaction((*txs)[index]);
    }
  }
}

double rand_val()
{
  return (double)rand() / (double)RAND_MAX;
}

/* From http://www.csee.usf.edu/~kchriste/tools/genzipf.c */
int zipf(double alpha, int n)
{
  static int first = TRUE;      // Static first time flag
  static double c = 0;          // Normalization constant
  double z;                     // Uniform random number (0 < z < 1)
  double sum_prob;              // Sum of probabilities
  double zipf_value;            // Computed exponential value to be returned
  int    i;                     // Loop counter

  // Compute normalization constant on first call only
  if (first == TRUE)
  {
    for (i=1; i<=n; i++)
      c = c + (1.0 / pow((double) i, alpha));
    c = 1.0 / c;
    first = FALSE;
  }

  // Pull a uniform random number (0 < z < 1)
  do
  {
    z = rand_val();
  }
  while ((z == 0) || (z == 1));

  // Map z to the value
  sum_prob = 0;
  for (i=1; i<=n; i++)
  {
    sum_prob = sum_prob + c / pow((double) i, alpha);
    if (sum_prob >= z)
    {
      zipf_value = i;
      break;
    }
  }

  // Assert that zipf_value is between 1 and N
  assert((zipf_value >=1) && (zipf_value <= n));

  return(zipf_value);
}

void BenchmarkTest::RunWorkload(Workload workload, Commit commit)
{
  // Setup
  uint64_t expected_vals[_num_records];
  for (auto i = 0; i < _num_records; i++) {
    expected_vals[i] = 0;
  }
  vector<shared_ptr<Transaction>> txs;
  vector<shared_ptr<int>> write_sets;
  
  // Generate workload
  for (auto i = 0; i < _num_txs; i++) {
    int index;
    if (workload == UNIFORM) {
      index = rand() % _num_records;
    } else if (workload == ZIPFIAN) {
      double alpha = 1.0;
      index = zipf(alpha, _num_records) - 1;
    }
    expected_vals[index] += 1;
    write_sets.push_back(std::make_shared<int>(index));
    txs.push_back(_lck_db->start_transaction());
  }
 
  // Execute
  thread workers[_num_threads];
  atomic<int> tx_counter = 0;

  // Start threads
  auto start = std::chrono::system_clock::now();

  for (auto i = 0; i < _num_threads; i++) {
    bool use_deps = commit ? true : false;
    workers[i] = std::thread(&thread_fn, &tx_counter, _lck_db.get(), _num_txs, &txs, &write_sets, use_deps);
  }

  // Block until done
  for (auto i = 0; i < _num_threads; i++) {
    workers[i].join();
  }

  auto end = std::chrono::system_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(end - start);
  int seconds = elapsed.count();
  // std::cout << elapsed.count() << "s \n";
  double throughput = (double) _num_txs / (double) seconds;
  std::cout << throughput << " TXes / sec \n";

  // Check records
  auto check_tx = _lck_db->start_transaction();
  for (auto i = 0; i < _num_records; i++) {
    DBKey k = {"test", to_string(i)};
    auto rec = static_pointer_cast<TestRecord>(_lck_db->read_ref(k, check_tx));
    ASSERT_EQ(rec->get(), expected_vals[i]);
  }
  _lck_db->commit_transaction(check_tx);
  
}

TEST_F(BenchmarkTest, UniformNaive)
{
  RunWorkload(UNIFORM, NAIVE);
}

TEST_F(BenchmarkTest, UniformDeps)
{
  RunWorkload(UNIFORM, DEPS);
}

TEST_F(BenchmarkTest, ZipfianNaive)
{
  RunWorkload(ZIPFIAN, NAIVE);
}

TEST_F(BenchmarkTest, ZipfianDeps)
{
  RunWorkload(ZIPFIAN, DEPS);
}

