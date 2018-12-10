#include <database.h>

Database::Database(unordered_map<string, shared_ptr<Table>> init)
{
  _tbl_map = init;
}


