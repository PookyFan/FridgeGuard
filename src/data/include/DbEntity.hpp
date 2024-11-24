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

    explicit DbEntity()
    {
        ids[0] = -1;
    }

    DbEntity(const DbEntity&) = default;

    DbEntity(DbEntity&&) = default;

    DbEntity(SchemaT&& data) : SchemaT(std::move(data))
    {
        ids[0] = -1;
    }

    template<class S = SchemaT>
    DbEntity(EntityPtr<typename S::FkEntity> fkEntity, SchemaT&& data)
        : SchemaT(std::move(data))
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
    using Base = DbEntity<ProductDescriptionSchema>;

    ProductDescription(Base&& entity) : Base(std::move(entity))
    {}

    ProductDescription(ProductDescriptionSchema&& data) : Base(std::move(data))
    {}

    ProductDescription(EntityPtr<ProductCategory> cat, ProductDescriptionSchema&& data)
        : Base(cat, std::move(data)), category(cat)
    {}

    void setFkEntity(EntityPtr<const ProductCategory> newCategory)
    {
        category = newCategory;
        setFkId(newCategory->getId());
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
    using Base = DbEntity<ProductInstanceSchema>;

    ProductInstance(Base&& entity) : Base(std::move(entity))
    {}

    ProductInstance(ProductInstanceSchema&& data) : Base(std::move(data))
    {}

    ProductInstance(EntityPtr<ProductDescription> desc, ProductInstanceSchema&& data)
        : Base(desc, std::move(data)), description(desc)
    {}

    void setFkEntity(EntityPtr<const ProductDescription> newDescription)
    {
        description = newDescription;
        setFkId(newDescription->getId());
    }

    EntityPtr<const ProductDescription> description;
};
}