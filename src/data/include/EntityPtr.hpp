#pragma once

#include <concepts>
#include <type_traits>

#include "EntityCache.hpp"

namespace FG::data
{
template<typename T>
class EntityPtr : public std::shared_ptr<T>
{
using Base = std::shared_ptr<T>;

public:
    EntityPtr() : Base(), cache(nullptr)
    {}

    EntityPtr(const Base& ptr, internal::EntityCache<T>& c) : Base(ptr), cache(&c)
    {}

    EntityPtr(EntityPtr&&) = default;
    EntityPtr(const EntityPtr&) = default;

    template<typename U>
    requires (std::same_as<std::remove_cv_t<T>, U> && !std::same_as<T, U>)
    EntityPtr(const EntityPtr<U>& other)
        : Base(other), cache(reinterpret_cast<const EntityPtr*>(&other)->cache)
    {
    }

    ~EntityPtr()
    {
        //Usage count is decreased in shared_ptr's destructor,
        //so value of 2 would now indicate that this and cache-hosted
        //entity pointers are the last ones, and we can remove it from cache safely
        if(Base::use_count() == 2)
            cache->erase(*static_cast<Base*>(this));
    }

    EntityPtr& operator=(EntityPtr&&) = default;
    EntityPtr& operator=(const EntityPtr&) = default;

    auto getCache() const
    {
        return cache;
    }

private:
    internal::EntityCache<T>* cache;
};
}