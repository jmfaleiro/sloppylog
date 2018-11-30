#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <lock_manager.h>

using namespace std;

class LogRecord {
};

struct RecordMetadata {
  size_t      table_id;
  size_t      num_fields;
};

class Record {

private:
  shared_ptr<RecordMetadata>    _meta_data;
  shared_ptr<size_t>            _field_offsets;
  shared_ptr<char>              _data;

public:
  
  /* Used for checkpointing. */
  virtual char* Serialize() = 0;
  virtual void Deserialize(char *data) = 0;
};

class Table {
public:
  virtual shared_ptr<Record> get_ref(DBKey key) = 0;
};

class Transaction {
protected:
  unordered_map<string, unique_ptr<LogRecord>>     _write_log;  
  unordered_set<shared_ptr<Transaction>>           _commit_deps;

public:
  virtual ~Transaction() {}
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
