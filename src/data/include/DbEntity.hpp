#pragma once

#include <chrono>
#include <memory>
#include <optional>
#include <string>

namespace FG::data
{
using Id = int;

using Datetime = std::chrono::time_point<std::chrono::system_clock>;

template<typename T>
using Nullable = std::optional<T>;

template<typename T>
using EntityPtr = std::shared_ptr<T>;

struct DbEntity
{
    Id id = -1;
};

struct ProductCategory : DbEntity
{
    std::string name;
    Nullable<std::string> imagePath;
    bool isArchived;
};

struct ProductDescription : DbEntity
{
    Id categoryId;
    std::string name;
    Nullable<std::string> barcode;
    unsigned int daysValidSuggestion;
    Nullable<std::string> imagePath;
    bool isArchived;

    const EntityPtr<ProductCategory> category;
};

struct ProductInstance : DbEntity
{
    Id descriptionId;
    Datetime purchaseDate;
    Datetime expirationDate;
    Nullable<unsigned int> daysToExpireWhenOpened;
    bool isOpen;
    bool isConsumed;

    const EntityPtr<ProductDescription> description;
};
}