#include "ProductDatabase.hpp"

namespace FG::data
{
ProductDatabase::ProductDatabase(const std::string& dbFilePath)
    : Base(), storage(internal::makeStorage(dbFilePath))
{
    storage.sync_schema();
}
}