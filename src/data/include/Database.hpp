#pragma once

#include <set>
#include <tuple>

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

template<typename... Entities>
class Database;

using ProductDatabase = Database<ProductCategory, ProductDescription, ProductInstance>;

template<typename... Entities>
class Database
{
public:
    Database(const std::string& dbFilePath = "");

    template<typename EntityT, typename... Args>
    EntityPtr<EntityT> create(Args... args)
    {
        assertEntityHandled<EntityT>();
        return insert(EntityPtr<EntityT>( new EntityT({args...}) ));
    }

    template<typename EntityT, typename FkEntityT, typename... Args>
    EntityPtr<EntityT> create(EntityPtr<FkEntityT> fkEntity, Args... args)
    {
        assertEntityHandled<EntityT>();
        assertEntityHandled<FkEntityT>();
        return insert(EntityPtr<EntityT>( new EntityT(fkEntity, {args...}) ));
    }

    template<typename EntityT>
    void commitChanges(EntityT& entity)
    {
        assertEntityHandled<EntityT>();
        storage.update(entity);
    }

    template<typename EntityT>
    void remove(EntityT& entity)
    {
        assertEntityHandled<EntityT>();
        storage.remove(entity);
    }

    template<typename EntityT>
    struct EntityComparator
    {
        using is_transparent = std::true_type;

        bool operator()(const Id& lhs, const EntityT& rhs) const
        {
            return lhs < rhs.getId();
        }

        bool operator()(const EntityT& lhs, const Id& rhs) const
        {
            return lhs.getId() < rhs;
        }

        bool operator()(const EntityT& lhs, const EntityT& rhs) const
        {
            return lhs.getId() < rhs.getId();
        }
    };

private:
    template<typename EntityT>
    void assertEntityHandled() noexcept
    {
        static_assert((std::is_same_v<EntityT, Entities> || ...), "Entity type not handled by database");
    }

    template<typename EntityT>
    EntityPtr<EntityT> insert(EntityPtr<EntityT>&& entity)
    {
        auto id = storage.insert(entity->asDbEntity());
        entity->setId(id);
        return entity;
    }

    using StorageT = decltype(internal::makeStorage());
    StorageT storage;
    std::tuple<std::set<Entities, EntityComparator<Entities>>...> caches;
};
}