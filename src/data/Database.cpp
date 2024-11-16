#include "Database.hpp"

namespace FG::data
{
Database::Database(const std::string& dbFilePath)
    : storage(internal::makeStorage(dbFilePath))
{
    storage.sync_schema();
}
}