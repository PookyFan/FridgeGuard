#include <array>
#include <gtest/gtest.h>
#include "ProductDatabase.hpp"

using namespace testing;

namespace FG::data::test
{
namespace
{
    std::array sampleProductCategories = {
        ProductCategory({ .name = "cat1", .imagePath = "path/to/image1", .isArchived = true }),
        ProductCategory({ .name = "cat2", .imagePath = std::nullopt, .isArchived = false })
    };

    std::array sampleProductDescriptions = {
        ProductDescription({ .name = "prod1", .barcode = "12345", .daysValidSuggestion = 3, .imagePath = "path/to/image1", .isArchived = true}),
        ProductDescription({ .name = "prod2", .barcode = "22456", .daysValidSuggestion = 4, .imagePath = std::nullopt, .isArchived = false}),
        ProductDescription({ .name = "prod3", .barcode = std::nullopt, .daysValidSuggestion = 1, .imagePath = "path/to/image3", .isArchived = true}),
        ProductDescription({ .name = "prod4", .barcode = std::nullopt, .daysValidSuggestion = 0, .imagePath = std::nullopt, .isArchived = false})
    };

    std::array sampleProductInstances = {
        ProductInstance({
            .purchaseDate = parseIsoDate("2023-01-17"), .expirationDate = parseIsoDate("2024-12-31"),
            .daysToExpireWhenOpened = 3, .isOpen = true, .isConsumed = true }),
        ProductInstance({
            .purchaseDate = parseIsoDate("2024-11-17"), .expirationDate = parseIsoDate("2024-12-17"),
            .daysToExpireWhenOpened = 3, .isOpen = true, .isConsumed = false }),
        ProductInstance({
            .purchaseDate = parseIsoDate("2022-02-25"), .expirationDate = parseIsoDate("2023-02-01"),
            .daysToExpireWhenOpened = 2, .isOpen = false, .isConsumed = true }),
        ProductInstance({
            .purchaseDate = parseIsoDate("2024-11-01"), .expirationDate = parseIsoDate("2024-12-15"),
            .daysToExpireWhenOpened = 2, .isOpen = false, .isConsumed = false }),
        ProductInstance({
            .purchaseDate = parseIsoDate("2021-06-19"), .expirationDate = parseIsoDate("2021-12-31"),
            .daysToExpireWhenOpened = std::nullopt, .isOpen = true, .isConsumed = true }),
        ProductInstance({
            .purchaseDate = parseIsoDate("2024-10-11"), .expirationDate = parseIsoDate("2024-11-25"),
            .daysToExpireWhenOpened = std::nullopt, .isOpen = true, .isConsumed = false }),
        ProductInstance({
            .purchaseDate = parseIsoDate("2024-10-12"), .expirationDate = parseIsoDate("2024-10-22"),
            .daysToExpireWhenOpened = std::nullopt, .isOpen = false, .isConsumed = true }),
        ProductInstance({
            .purchaseDate = parseIsoDate("2024-11-17"), .expirationDate = parseIsoDate("2024-12-17"),
            .daysToExpireWhenOpened = std::nullopt, .isOpen = false, .isConsumed = false })
    };
}

struct ProductDatabaseTestFixture : public Test
{
    void assertProductCategoriesAreEqual(const ProductCategory& lhs, const ProductCategory& rhs)
    {
        ASSERT_EQ(lhs.name, rhs.name);
        ASSERT_EQ(lhs.imagePath, rhs.imagePath);
        ASSERT_EQ(lhs.isArchived, rhs.isArchived);
    }

    void assertProductDescriptionsAreEqual(const ProductDescription& lhs, const ProductDescription& rhs)
    {
        ASSERT_EQ(lhs.name, rhs.name);
        ASSERT_EQ(lhs.barcode, rhs.barcode);
        ASSERT_EQ(lhs.daysValidSuggestion, rhs.daysValidSuggestion);
        ASSERT_EQ(lhs.imagePath, rhs.imagePath);
        ASSERT_EQ(lhs.isArchived, rhs.isArchived);
    }

    void assertProductInstancesAreEqual(const ProductInstance& lhs, const ProductInstance& rhs)
    {
        ASSERT_EQ(lhs.purchaseDate, rhs.purchaseDate);
        ASSERT_EQ(lhs.expirationDate, rhs.expirationDate);
        ASSERT_EQ(lhs.daysToExpireWhenOpened, rhs.daysToExpireWhenOpened);
        ASSERT_EQ(lhs.isOpen, rhs.isOpen);
        ASSERT_EQ(lhs.isConsumed, rhs.isConsumed);
    }

    ProductDatabase db{};
};

TEST_F(ProductDatabaseTestFixture, ProductDatabaseShouldStoreAndRetrieveEntities)
{
    for(const auto& templCat : sampleProductCategories)
    {
        auto newCat = db.create<ProductCategory>(templCat.name, templCat.imagePath, templCat.isArchived);
        assertProductCategoriesAreEqual(templCat, *newCat);
    }

    int count = 0;
    for(const auto& templDesc : sampleProductDescriptions)
    {
        auto category = db.retrieve<ProductCategory>(++count);
        auto newDesc = db.create<ProductDescription>(
            category, templDesc.name, templDesc.barcode,
            templDesc.daysValidSuggestion,
            templDesc.imagePath, templDesc.isArchived);
        assertProductDescriptionsAreEqual(templDesc, *newDesc);
        count %= sampleProductCategories.size();
    }

    count = 0;
    for(const auto& templInst : sampleProductInstances)
    {
        auto description = db.retrieve<ProductDescription>(++count);
        auto newInst = db.create<ProductInstance>(
            description, templInst.purchaseDate, templInst.expirationDate,
            templInst.daysToExpireWhenOpened, templInst.isOpen, templInst.isConsumed
        );
        assertProductInstancesAreEqual(templInst, *newInst);
        count %= sampleProductDescriptions.size();
    }

    count = 0;
    for(const auto& templInst : sampleProductInstances)
    {
        auto inst = db.retrieve<ProductInstance>(++count);
        auto& templDesc = sampleProductDescriptions.at((count - 1) % sampleProductDescriptions.size());
        auto& templCat = sampleProductCategories.at((count - 1) % sampleProductCategories.size());
        assertProductInstancesAreEqual(templInst, *inst);
        ASSERT_EQ(inst->getFkId(), inst->description->getId());
        assertProductDescriptionsAreEqual(templDesc, *inst->description);
        ASSERT_EQ(inst->description->getFkId(), inst->description->category->getId());
        assertProductCategoriesAreEqual(templCat, *inst->description->category);
    }
}

/* Generic entities management tests */

template<typename T>
struct TypedProductDatabaseTestFixture : ProductDatabaseTestFixture
{};

using EntitiesTypes = Types<ProductCategory, ProductDescription, ProductInstance>;
TYPED_TEST_SUITE(TypedProductDatabaseTestFixture, EntitiesTypes);

TYPED_TEST(TypedProductDatabaseTestFixture, ProductDatabaseShouldCreateEntitiesWithIncreasingIds)
{
    constexpr auto numOfEntities = 100;
    for(auto i = 1; i <= numOfEntities; ++i)
        ASSERT_EQ(i, this->db.template create<TypeParam>()->getId());
}

}