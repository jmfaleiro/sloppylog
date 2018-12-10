#include <simple_table.h>

SimpleTable::SimpleTable()
{
}

shared_ptr<Record> SimpleTable::get_ref(DBKey key)
{
  SimpleHashTable::accessor ac;
  if (!_hash_tbl.find(ac, key)) {
    return nullptr;
  }
  return ac->second;
}

bool SimpleTable::insert(DBKey key, shared_ptr<Record> record)
{
  SimpleHashTable::accessor ac;
  if (!_hash_tbl.insert(ac, key)) {
    return false;
  }
  ac->second = record; 
  return true;
}

bool SimpleTable::remove(DBKey key)
{
  return _hash_tbl.erase(key);
}
