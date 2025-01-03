#pragma once

#include <array>
#include <optional>
#include <string>
#include <utility>

#include "DatetimeUtils.hpp"
#include "EntityPtr.hpp"

namespace FG::data
{
template<class SchemaT>
class DbEntity : public SchemaT
{
public:
    using DbEntityType = DbEntity;
    using SchemaType = SchemaT;

    explicit DbEntity() : DbEntity(SchemaT{})
    {
    }

    DbEntity(const DbEntity&) = default;

    DbEntity(SchemaT&& data) : SchemaT(std::move(data)), ids({uninitializedId}), isDeleted(false)
    {
    }

    template<class S = SchemaT>
    DbEntity(EntityPtr<typename S::FkEntity> fkEntity, SchemaT&& data)
        : SchemaT(std::move(data)), ids({uninitializedId, fkEntity->getId()}), isDeleted(false)
    {
    }

    DbEntity& operator=(const DbEntity&) = default;

    DbEntity& operator=(DbEntity&&) = default;

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
        if(ids[0] <= uninitializedId)
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
    explicit ProductDescription() : DbEntityType()
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
    explicit ProductInstance() : DbEntityType()
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