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

template<class, typename = void>
struct HasFkEntity : std::false_type {};

template<class T>
struct HasFkEntity<T, std::void_t<typename T::FkEntity>> : std::true_type {};

template<class S, std::enable_if_t<!HasFkEntity<S>::value, bool> = false>
constexpr auto primaryAndForeignKeysCount() { return 1; }

template<class S, std::enable_if_t<HasFkEntity<S>::value, bool> = true>
constexpr auto primaryAndForeignKeysCount() { return 2; }

template<class SchemaT>
class DbEntity : public SchemaT
{
public:
    DbEntity(SchemaT&& data) : SchemaT(std::move(data))
    {
        ids[0] = -1;
    }

    template<class S = SchemaT, std::enable_if_t<HasFkEntity<S>::value, bool> = true>
    DbEntity(EntityPtr<typename S::FkEntity> fkEntity, SchemaT&& data) : SchemaT(std::move(data))
    {
        ids[0] = -1;
        ids[1] = fkEntity->getId();
    }

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

    template<class S = SchemaT>
    const std::enable_if_t<HasFkEntity<S>::value, Id>
    getFkId() const
    {
        return ids[1];
    }

    template<class S = SchemaT>
    std::enable_if_t<HasFkEntity<S>::value>
    setFkId(Id newId)
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

    ProductDescription(ProductDescriptionSchema&& data) : Base(std::move(data))
    {}

    ProductDescription(EntityPtr<ProductCategory> cat, ProductDescriptionSchema&& data)
         : Base(cat, std::move(data)), category(cat)
    {}

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

    ProductInstance(ProductInstanceSchema&& data) : Base(std::move(data))
    {}

    ProductInstance(EntityPtr<ProductDescription> desc, ProductInstanceSchema&& data)
         : Base(desc, std::move(data)), description(desc)
    {}

    EntityPtr<const ProductDescription> description;
};
}