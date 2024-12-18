#pragma once

#include <memory>
#include <set>

#include "EntityUtils.hpp"

namespace FG::data
{
namespace internal
{
template<typename EntityT>
struct EntityComparator
{
    using is_transparent = std::true_type;

    bool operator()(const Id& lhs, const std::shared_ptr<EntityT>& rhs) const
    {
        return lhs < rhs->getId();
    }

    bool operator()(const std::shared_ptr<EntityT>& lhs, const Id& rhs) const
    {
        return lhs->getId() < rhs;
    }

    bool operator()(const std::shared_ptr<EntityT>& lhs, const std::shared_ptr<EntityT>& rhs) const
    {
        if(lhs->getId() == uninitializedId) return false;
        if(rhs->getId() == uninitializedId) return true;
        return lhs->getId() < rhs->getId();
    }
};

template<typename EntityT>
using EntityCache = std::set<std::shared_ptr<EntityT>, EntityComparator<EntityT>>;
}
}