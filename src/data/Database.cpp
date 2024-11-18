#include "Database.hpp"

namespace FG::data
{
template<typename... Entities>
Database<Entities...>::Database(const std::string& dbFilePath)
    : storage(internal::makeStorage(dbFilePath))
{
    storage.sync_schema();
}

template class Database<ProductCategory, ProductDescription, ProductInstance>;
}