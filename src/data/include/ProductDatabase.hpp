#pragma once

#include "Database.hpp"

namespace FG::data
{
namespace internal
{
inline auto makeStorage(const std::string& dbFilePath = "")
{
    using namespace sqlite_orm;
    return make_storage(
        dbFilePath,
        make_table("categories",
            make_column("id", &ProductCategory::getId, &ProductCategory::setId, primary_key().autoincrement()),
            make_column("name", &ProductCategory::name),
            make_column("imagePath", &ProductCategory::imagePath),
            make_column("isArchived", &ProductCategory::isArchived)
        ),
        make_table("descriptions",
            make_column("id", &ProductDescription::getId, &ProductDescription::setId, primary_key().autoincrement()),
            make_column("categoryId", &ProductDescription::getFkId, &ProductDescription::setFkId),
            make_column("name", &ProductDescription::name),
            make_column("barcode", &ProductDescription::barcode),
            make_column("daysValidSuggestion", &ProductDescription::daysValidSuggestion),
            make_column("imagePath", &ProductDescription::imagePath),
            make_column("isArchived", &ProductDescription::isArchived)
        ),
        make_table("instances",
            make_column("id", &ProductInstance::getId, &ProductInstance::setId, primary_key().autoincrement()),
            make_column("descriptionId", &ProductInstance::getFkId, &ProductInstance::setFkId),
            make_column("purchaseDate", &ProductInstance::getPurchaseDateTimestamp, &ProductInstance::setPurchaseDateTimestamp),
            make_column("expirationDate", &ProductInstance::getExpirationDateTimestamp, &ProductInstance::setExpirationDateTimestamp),
            make_column("daysToExpireWhenOpened", &ProductInstance::daysToExpireWhenOpened),
            make_column("isOpen", &ProductInstance::isOpen),
            make_column("isConsumed", &ProductInstance::isConsumed)
        )
    );
}
}

class ProductDatabase : public Database<ProductDatabase, ProductCategory, ProductDescription, ProductInstance>
{
friend class Database<ProductDatabase, ProductCategory, ProductDescription, ProductInstance>;

public:
    ProductDatabase(const std::string& dbFilePath = "");

private:
    using Base = Database<ProductDatabase, ProductCategory, ProductDescription, ProductInstance>;
    using StorageT = decltype(internal::makeStorage());
    
    template<typename EntityT>
    void insertImpl(EntityT& entity)
    {
        auto id = storage.insert(entity.asDbEntity());
        entity.setId(id);
    }

    template<typename EntityT>
    EntityT retrieveImpl(Id id)
    {
        return EntityT(storage.get<typename EntityT::DbEntityType>(id));
    }

    StorageT storage;
};
}