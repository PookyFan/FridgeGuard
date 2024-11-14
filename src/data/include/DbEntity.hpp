#pragma once

#include <chrono>
#include <optional>
#include <string>

namespace FG::data
{
using Id = unsigned int;

using Datetime = std::chrono::time_point<std::chrono::system_clock>;

template<typename T>
using Nullable = std::optional<T>;

struct DbEntity
{
    Id id;
};

struct ProductCategory : DbEntity
{
    std::string name;
    Nullable<std::string> imagePath;
    bool isArchived;
};

struct ProductDescription : DbEntity
{
    std::string name;
    Id categoryId;
    Nullable<std::string> barcode;
    unsigned int daysValidSuggestion;
    Nullable<std::string> imagePath;
    bool isArchived;

    const ProductCategory& category;
};

struct ProductInstance : DbEntity
{
    Id descriptionId;
    Datetime purchaseDate;
    Datetime expirationDate;
    Nullable<unsigned int> daysToExpireWhenOpened;
    bool isOpen;
    bool isConsumed;

    const ProductDescription& description;
};
}