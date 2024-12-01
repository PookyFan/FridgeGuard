#pragma once

#include <set>
#include <tuple>

#include <sqlite_orm/sqlite_orm.h>
#include "DbEntity.hpp"

namespace FG::data
{
namespace internal
{
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
        if(lhs.getId() == -1) return false;
        if(rhs.getId() == -1) return true;
        return lhs.getId() < rhs.getId();
    }
};

template<typename EntityT>
using EntityCache = std::set<EntityT, EntityComparator<EntityT>>;

template<typename EntityT>
class EntityRemover
{
    using CacheT = EntityCache<EntityT>;

public:
    EntityRemover(CacheT& c) : cache(c)
    {}

    EntityRemover(const EntityRemover&) = default;

    void operator()(EntityT* entity) noexcept
    {
        auto it = cache.find(entity->getId());
        if(it != cache.end())
            cache.erase(it);
    }

private:
    CacheT& cache;
};
}

template<class DbImpl, typename... Entities>
class Database
{
public:
    template<typename EntityT, typename... Args>
    EntityPtr<EntityT> create(Args... args)
    {
        auto& cache = getCache<EntityT>();
        auto& entity = const_cast<EntityT&>( *cache.insert(cache.end(), EntityT(typename EntityT::SchemaType{args...})) );
        EntityPtr<EntityT> entityPtr(&entity, internal::EntityRemover(cache));
        getImpl().insertImpl(*entityPtr);
        return entityPtr;
    }

    template<typename EntityT, typename FkEntityT, typename... Args>
    EntityPtr<EntityT> create(EntityPtr<FkEntityT> fkEntity, Args... args)
    {
        auto& cache = getCache<EntityT>();
        auto& entity = const_cast<EntityT&>( *cache.insert(cache.end(), EntityT(fkEntity, {args...})) );
        EntityPtr<EntityT> entityPtr(&entity, internal::EntityRemover(cache));
        getImpl().insertImpl(*entityPtr);
        return entityPtr;
    }

    template<typename EntityT>
    EntityPtr<EntityT> retrieve(Id id)
    {
        auto& cache = getCache<EntityT>();
        auto entityIt = cache.find(id);
        EntityT* entityPtr;
        if(entityIt != cache.end() && entityIt->isValid())
            entityPtr = &const_cast<EntityT&>(*entityIt);
        else
        {
            auto& entity = const_cast<EntityT&>( *cache.insert(cache.end(), retrieveFromDb<EntityT>(id)) );
            entityPtr = &entity;
        }
        return EntityPtr<EntityT>(entityPtr, internal::EntityRemover(cache));
    }

    template<WithFkEntity EntityT>
    void commitChanges(EntityT& entity)
    { 
        assertEntityInCache(entity);
        entity.updateFkId();
        getImpl().updateImpl(entity);
    }

    template<typename EntityT>
    void commitChanges(const EntityT& entity)
    { 
        assertEntityInCache(entity);
        getImpl().updateImpl(entity);
    }

    template<typename EntityT>
    void remove(EntityPtr<EntityT>&& entity)
    {
        assertEntityInCache(*entity);

        EntityPtr<EntityT> entityPtr(std::move(entity));
        getImpl().removeImpl(*entityPtr);
        entityPtr->invalidate();
    }

private:
    DbImpl& getImpl()
    {
        return *static_cast<DbImpl*>(this);
    }

    template<typename EntityT>
    void assertEntityInCache(const EntityT& entity)
    {
        auto& cache = getCache<EntityT>();
        if(cache.find(entity) == cache.end())
            throw std::runtime_error("Updated entity not found in cache");
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

    std::tuple<std::set<Entities, internal::EntityComparator<Entities>>...> caches;
};
}