#pragma once

#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace FG::data
{
using Id = int;

using Datetime = std::chrono::time_point<std::chrono::system_clock>;

template<typename T>
using Nullable = std::optional<T>;

template<typename T>
using EntityPtr = std::shared_ptr<T>;

template<class SchemaT>
class DbEntity : public SchemaT
{
public:
    DbEntity(SchemaT&& data) : SchemaT(std::move(data))
    {}

    Id getId() { return id; }
    void setId(Id newId)
    {
        if(id <= 0)
            id = newId;
        else
            throw std::runtime_error("Tried to change ID for already existing entity");
    }

private:
    Id id = -1;
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
    Id categoryId;

    std::string name;
    Nullable<std::string> barcode;
    unsigned int daysValidSuggestion;
    Nullable<std::string> imagePath;
    bool isArchived;

    const EntityPtr<ProductCategory> category;
};

using ProductDescription = DbEntity<ProductDescriptionSchema>;

struct ProductInstanceSchema
{
    Id descriptionId;

    // Datetime purchaseDate;
    // Datetime expirationDate;
    Nullable<unsigned int> daysToExpireWhenOpened;
    bool isOpen;
    bool isConsumed;

    const EntityPtr<ProductDescription> description;
};

using ProductInstance = DbEntity<ProductInstanceSchema>;
}