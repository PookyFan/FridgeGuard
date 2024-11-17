#pragma once

#include <sqlite_orm/sqlite_orm.h>
#include "DbEntity.hpp"

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

class Database
{
public:
    Database(const std::string& dbFilePath = "");

    template<typename EntityT, typename... Args>
    EntityPtr<EntityT> create(Args... args)
    {
        return insert(EntityPtr<EntityT>( new EntityT({args...}) ));
    }

    template<typename EntityT, typename FkEntityT, typename... Args>
    EntityPtr<EntityT> create(EntityPtr<FkEntityT> fkEntity, Args... args)
    {
        return insert(EntityPtr<EntityT>( new EntityT(fkEntity, {args...}) ));
    }

    template<typename EntityT>
    void commitChanges(EntityT& entity)
    {
        storage.update(entity);
    }

    template<typename EntityT>
    void remove(EntityT& entity)
    {
        storage.remove(entity);
    }

private:
    template<typename EntityT>
    EntityPtr<EntityT> insert(EntityPtr<EntityT>&& entity)
    {
        auto id = storage.insert(entity->asDbEntity());
        entity->setId(id);
        return entity;
    }

    using StorageT = decltype(internal::makeStorage());
    StorageT storage;
};
}