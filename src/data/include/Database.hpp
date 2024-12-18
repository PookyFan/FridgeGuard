#pragma once

#include <algorithm>
#include <iterator>
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

    template<typename EntityT, typename ConditionT>
    std::vector<EntityPtr<EntityT>> retrieve(ConditionT&& cond)
    {
        auto& cache = getCache<EntityT>();
        auto entities = retrieveFromDb<EntityT>(forward(cond));
        std::vector<EntityPtr<EntityT>> entitiesPtrs;
        entitiesPtrs.reserve(entities.size());
        for(auto &e : entities)
        {
            const auto& [entityPtrIterator, _] = cache.insert(std::make_shared<EntityT>(std::move(e)));
            entitiesPtrs.emplace_back(*entityPtrIterator, cache);
        }

        return entitiesPtrs;
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
    template<typename T>
    T&& forward(T&& obj)
    {
        return std::move(obj);
    }

    template<typename T>
    const T& forward(T& obj)
    {
        return obj;
    }

    template<typename T>
    const T& forward(const T& obj)
    {
        return obj;
    }

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

    template<WithFkEntity EntityT, typename ConditionT>
    std::vector<EntityT> retrieveFromDb(ConditionT&& cond)
    {
        auto entities = getImpl().template retrieveImpl<EntityT>(forward(cond));
        std::set<Id> fkIds;
        std::transform(entities.begin(), entities.end(), std::inserter(fkIds, fkIds.end()),
            [](const auto& entity) { return entity.getFkId(); });

        auto fkEntitiesPtrs = retrieve<typename EntityT::FkEntity>(fkIds);
        for(auto& entity : entities)
        {
            const auto fkEntityPtrIt = std::lower_bound(
                fkEntitiesPtrs.begin(), fkEntitiesPtrs.end(), entity.getFkId(),
                [](const auto& fkEntityPtr, const Id id) { return fkEntityPtr->getId() < id; });
            if(fkEntityPtrIt == fkEntitiesPtrs.end())
                throw std::runtime_error("Foreign key entity not present in database");
            entity.setFkEntity(*fkEntityPtrIt);
        }

        return entities;
    }

    template<typename EntityT, typename ConditionT>
    std::vector<EntityT> retrieveFromDb(ConditionT&& cond)
    {
        return getImpl().template retrieveImpl<EntityT>(forward(cond));
    }

    template<typename EntityT>
    internal::EntityCache<EntityT>& getCache()
    {
        return std::get<internal::EntityCache<EntityT>>(caches);
    }

    std::tuple<internal::EntityCache<Entities>...> caches;
};
}