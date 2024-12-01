#pragma once

#include <array>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>

#include "DatetimeUtils.hpp"

namespace FG::data
{
using Id = int;

template<typename T>
using Nullable = std::optional<T>;

template<typename T>
using EntityPtr = std::shared_ptr<T>;

template<class T>
concept WithFkEntity = requires { typename T::FkEntity; };

template<class S>
constexpr auto primaryAndForeignKeysCount() { return 1; }

template<WithFkEntity S>
constexpr auto primaryAndForeignKeysCount() { return 2; }

template<class SchemaT>
class DbEntity : public SchemaT
{
public:
    using DbEntityType = DbEntity;
    using SchemaType = SchemaT;

    explicit DbEntity() : isDeleted(false)
    {
        ids[0] = -1;
    }

    DbEntity(const DbEntity&) = default;

    DbEntity(DbEntity&&) = default;

    DbEntity(SchemaT&& data) : SchemaT(std::move(data)), isDeleted(false)
    {
        ids[0] = -1;
    }

    template<class S = SchemaT>
    DbEntity(EntityPtr<typename S::FkEntity> fkEntity, SchemaT&& data)
        : SchemaT(std::move(data)), isDeleted(false)
    {
        ids[0] = -1;
        ids[1] = fkEntity->getId();
    }

    DbEntity& operator=(const DbEntity&) = default;

    DbEntity& operator=(DbEntity&&) = default;

    auto& asDbEntity()
    {
        return *this;
    }

    const auto& asDbEntity() const
    {
        return *this;
    }

    void invalidate()
    {
        isDeleted = true;
    }

    bool isValid() const
    {
        return !isDeleted;
    }

    const Id getId() const
    {
        return ids[0];
    }

    void setId(Id newId)
    {
        if(ids[0] <= 0)
            ids[0] = newId;
        else
            throw std::runtime_error("Tried to change ID for already existing entity");
    }

    const Id getFkId() const
    requires WithFkEntity<SchemaT>
    {
        return ids[1];
    }

    void setFkId(Id newId)
    requires WithFkEntity<SchemaT>
    {
        ids[1] = newId;
    }

private:
    std::array<Id, primaryAndForeignKeysCount<SchemaT>()> ids;
    bool isDeleted;
};

struct ProductCategorySchema
{
    std::string name;
    Nullable<std::string> imagePath;
    bool isArchived;
};

using ProductCategory = DbEntity<ProductCategorySchema>;

struct ProductDescriptionSchema
{
    using FkEntity = ProductCategory;

    std::string name;
    Nullable<std::string> barcode;
    unsigned int daysValidSuggestion;
    Nullable<std::string> imagePath;
    bool isArchived;
};

struct ProductDescription : public DbEntity<ProductDescriptionSchema>
{
    ProductDescription(DbEntityType&& entity) : DbEntityType(std::move(entity))
    {}

    ProductDescription(ProductDescriptionSchema&& data) : DbEntityType(std::move(data))
    {}

    ProductDescription(EntityPtr<ProductCategory> cat, ProductDescriptionSchema&& data)
        : DbEntityType(cat, std::move(data)), category(cat)
    {}

    void setFkEntity(EntityPtr<const ProductCategory> newCategory)
    {
        category = newCategory;
        setFkId(newCategory->getId());
    }

    void updateFkId()
    {
        setFkId(category->getId());
    }

    EntityPtr<const ProductCategory> category;
};

struct ProductInstanceSchema
{
    using FkEntity = ProductDescription;

    Datetime purchaseDate;
    Datetime expirationDate;
    Nullable<unsigned int> daysToExpireWhenOpened;
    bool isOpen;
    bool isConsumed;

    const Timestamp getPurchaseDateTimestamp() const
    {
        return datetimeToUnixTimestamp(purchaseDate);
    }

    void setPurchaseDateTimestamp(Timestamp newDate)
    {
        purchaseDate = unixTimestampToDatetime(newDate);
    }

    const Timestamp getExpirationDateTimestamp() const
    {
        return datetimeToUnixTimestamp(expirationDate);
    }

    void setExpirationDateTimestamp(Timestamp newDate)
    {
        expirationDate = unixTimestampToDatetime(newDate);
    }
};

struct ProductInstance : public DbEntity<ProductInstanceSchema>
{
    ProductInstance(DbEntityType&& entity) : DbEntityType(std::move(entity))
    {}

    ProductInstance(ProductInstanceSchema&& data) : DbEntityType(std::move(data))
    {}

    ProductInstance(EntityPtr<ProductDescription> desc, ProductInstanceSchema&& data)
        : DbEntityType(desc, std::move(data)), description(desc)
    {}

    void setFkEntity(EntityPtr<const ProductDescription> newDescription)
    {
        description = newDescription;
        setFkId(newDescription->getId());
    }

    void updateFkId()
    {
        setFkId(description->getId());
    }

    EntityPtr<const ProductDescription> description;
};
}