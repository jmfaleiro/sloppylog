#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <string>

using namespace std;

struct DBKey {
  string      table;
  string      record;

  bool operator==(const DBKey &other) const
  {
    return this->table == other.table && this->record == other.record;
  }

  static size_t hash(const DBKey& k)
  {
    auto table_hash = std::hash<string>{}(k.table);
    auto record_hash = std::hash<string>{}(k.record);
    return (table_hash << 1) ^ record_hash;
  }

  static bool equal(const DBKey& k1, const DBKey& k2)
  {
    return k1.table == k2.table && k1.record == k2.record;
  }
};

template<>
struct hash<DBKey>
{
  std::size_t operator()(const DBKey &k) const
  {
    return DBKey::hash(k);
  }
};


class LogRecord {
};

struct RecordMetadata {
  size_t      table_id;
  size_t      num_fields;
};

class Record {

protected:
  RecordMetadata                _meta_data;
  size_t                        *_field_offsets;
  char                          *_data;

public:
};

typedef enum {
  OTHER = 0,
  COMMITTED = 1
} Status;

class Table {
public:
  virtual shared_ptr<Record>      get_ref(DBKey key) = 0;
  virtual bool                    insert(DBKey key, shared_ptr<Record> record) = 0; 
  virtual bool                    remove(DBKey key) = 0;
};

class Transaction {
protected:
  unordered_map<string, unique_ptr<LogRecord>>     _write_log;

public:
  virtual ~Transaction() {}
  unordered_set<shared_ptr<Transaction>>           _commit_deps;
  Status                                          status;

};

class Database {

protected:
  unordered_map<string, shared_ptr<Table>>     _tbl_map;       

public:
  Database(unordered_map<string, shared_ptr<Table>> init);

  /* XXX Need to figure out how UNDO logging is coordinated with writing. */
  virtual shared_ptr<Record>    read_ref(DBKey key, shared_ptr<Transaction> tx) = 0;
  virtual shared_ptr<Record>    write_ref(DBKey key, shared_ptr<Transaction> tx) = 0;

  virtual shared_ptr<Transaction> start_transaction() = 0; 
  virtual bool commit_transaction(shared_ptr<Transaction> tx) = 0;
};
