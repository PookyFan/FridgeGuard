#pragma once

#include <tuple>
#include <vector>

#include <sqlite_orm/sqlite_orm.h>
#include "DbEntity.hpp"

namespace FG::data
{
template<class DbImpl, typename... Entities>
class Database
{
public:
    template<typename EntityT, typename... Args>
    EntityPtr<EntityT> create(Args... args)
    {
        auto& cache = getCache<EntityT>();
        const auto& entityPtr = *cache.insert(cache.end(), std::make_shared<EntityT>(typename EntityT::SchemaType{args...}));
        getImpl().insertImpl(*entityPtr);
        return {entityPtr, cache};
    }

    template<typename EntityT, typename FkEntityT, typename... Args>
    EntityPtr<EntityT> create(EntityPtr<FkEntityT> fkEntity, Args... args)
    {
        auto& cache = getCache<EntityT>();
        auto& entityPtr = *cache.insert(cache.end(), std::make_shared<EntityT>(fkEntity, typename EntityT::SchemaType{args...}));
        getImpl().insertImpl(*entityPtr);
        return {entityPtr, cache};
    }

    template<typename EntityT>
    EntityPtr<EntityT> retrieve(Id id)
    {
        auto& cache = getCache<EntityT>();
        auto entityIt = cache.find(id);
        if(entityIt != cache.end() && (*entityIt)->isValid())
            return {*entityIt, cache};

        auto& entityPtr = *cache.insert(cache.end(), std::make_shared<EntityT>(retrieveFromDb<EntityT>(id)));
        return {entityPtr, cache};
    }

    template<WithFkEntity EntityT>
    void commitChanges(EntityPtr<EntityT>& entity)
    { 
        assertEntityInCache(entity);
        entity->updateFkId();
        getImpl().updateImpl(*entity);
    }

    template<typename EntityT>
    void commitChanges(const EntityPtr<EntityT>& entity)
    { 
        assertEntityInCache(entity);
        getImpl().updateImpl(*entity);
    }

    template<typename EntityT>
    void remove(EntityPtr<EntityT>&& entity)
    {
        assertEntityInCache(entity);

        auto& cache = getCache<EntityT>();
        EntityPtr<EntityT> invalidatedEntity(std::move(entity));
        getImpl().removeImpl(*invalidatedEntity);
        invalidatedEntity->invalidate();
        cache.erase(invalidatedEntity);
    }

private:
    DbImpl& getImpl()
    {
        return *static_cast<DbImpl*>(this);
    }

    template<typename EntityT>
    void assertEntityInCache(const EntityPtr<EntityT>& entity)
    {
        auto& cache = getCache<EntityT>();
        if(cache.find(entity) == cache.end())
            throw std::runtime_error("Entity not found in cache");
    }

    template<WithFkEntity EntityT>
    EntityT retrieveFromDb(Id id)
    {
        auto entity = getImpl().template retrieveImpl<EntityT>(id);
        auto fkEntityPtr = retrieve<typename EntityT::FkEntity>(entity.getFkId());
        entity.setFkEntity(fkEntityPtr);
        return entity;
    }

    template<typename EntityT>
    EntityT retrieveFromDb(Id id)
    {
        return getImpl().template retrieveImpl<EntityT>(id);
    }

    template<typename EntityT>
    internal::EntityCache<EntityT>& getCache()
    {
        return std::get<internal::EntityCache<EntityT>>(caches);
    }

    std::tuple<internal::EntityCache<Entities>...> caches;
};
}