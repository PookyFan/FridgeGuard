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

struct ProductCategory
{
    Id id;
    std::string name;
    Nullable<std::string> imagePath;
    bool isArchived;
};

struct ProductDescription
{
    Id id;
    Id categoryId;
    std::string name;
    Nullable<std::string> barcode;
    unsigned int daysValidSuggestion;
    Nullable<std::string> imagePath;
    bool isArchived;

    const EntityPtr<ProductCategory> category;
};

struct ProductInstance
{
    Id id;
    Id descriptionId;
    // Datetime purchaseDate;
    // Datetime expirationDate;
    Nullable<unsigned int> daysToExpireWhenOpened;
    bool isOpen;
    bool isConsumed;

    const EntityPtr<ProductDescription> description;
};
}