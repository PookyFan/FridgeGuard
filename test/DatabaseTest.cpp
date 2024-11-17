#include <vector>

#include <gtest/gtest.h>
#include "Database.hpp"

using namespace testing;

namespace FG::data::test
{
struct DatabaseTestFixture : public Test
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
        ASSERT_EQ(lhs.getFkId(), rhs.getFkId());
        ASSERT_EQ(lhs.category.get(), rhs.category.get());
    }

    void assertProductInstancesAreEqual(const ProductInstance& lhs, const ProductInstance& rhs)
    {
        ASSERT_EQ(lhs.purchaseDate, rhs.purchaseDate);
        ASSERT_EQ(lhs.expirationDate, rhs.expirationDate);
        ASSERT_EQ(lhs.daysToExpireWhenOpened, rhs.daysToExpireWhenOpened);
        ASSERT_EQ(lhs.isOpen, rhs.isOpen);
        ASSERT_EQ(lhs.isConsumed, rhs.isConsumed);
        ASSERT_EQ(lhs.getFkId(), rhs.getFkId());
        ASSERT_EQ(lhs.description.get(), rhs.description.get());
    }

    Database db{};
};

/* Generic entities management tests */

template<typename T>
struct TypedDatabaseTestFixture : DatabaseTestFixture
{};

using EntitiesTypes = Types<ProductCategory, ProductDescription, ProductInstance>;
TYPED_TEST_SUITE(TypedDatabaseTestFixture, EntitiesTypes);

TYPED_TEST(TypedDatabaseTestFixture, DatabaseShouldCreateEntitiesWithIncreasingIds)
{
    constexpr auto numOfEntities = 100;
    for(auto i = 1; i <= numOfEntities; ++i)
        ASSERT_EQ(i, this->db.template create<TypeParam>()->getId());
}

TYPED_TEST(TypedDatabaseTestFixture, EntityShouldThrowWhenAttemptingToChangeItsIdAfterCreationByDatabase)
{
    auto entity = this->db.template create<TypeParam>();
    ASSERT_THROW(entity->setId(2), std::runtime_error);
}

/* Tests for ProductCategory entities management */

struct ProductCategoryDatabaseTestFixture : DatabaseTestFixture, public WithParamInterface<ProductCategory>
{};

TEST_P(ProductCategoryDatabaseTestFixture, DatabaseShouldCreateProductCategoryAccordingToCreateMethodParameters)
{
    auto templCat = GetParam();
    auto newCat = db.create<ProductCategory>(templCat.name, templCat.imagePath, templCat.isArchived);
    assertProductCategoriesAreEqual(templCat, *newCat);
}

INSTANTIATE_TEST_SUITE_P(ProductCategoryTest, ProductCategoryDatabaseTestFixture, Values(
    ProductCategory({ .name = "cat1", .imagePath = "path/to/image1", .isArchived = true }),
    ProductCategory({ .name = "cat2", .imagePath = "path/to/image2", .isArchived = false }),
    ProductCategory({ .name = "cat3", .imagePath = std::nullopt, .isArchived = true }),
    ProductCategory({ .name = "cat4", .imagePath = std::nullopt, .isArchived = false })
));

/* Tests for ProductDescription entities management */

struct ProductDescriptionDatabaseTestFixture : DatabaseTestFixture, public WithParamInterface<ProductDescription>
{};

TEST_P(ProductDescriptionDatabaseTestFixture, DatabaseShouldCreateProductDescriptionAccordingToCreateMethodParameters)
{
    auto templDesc = GetParam();
    auto category = db.create<ProductCategory>();
    templDesc.setFkId(category->getId());
    templDesc.category = category;
    auto newDesc = db.create<ProductDescription>(
        category, templDesc.name, templDesc.barcode,
        templDesc.daysValidSuggestion,
        templDesc.imagePath, templDesc.isArchived);
    assertProductDescriptionsAreEqual(templDesc, *newDesc);
}

INSTANTIATE_TEST_SUITE_P(ProductDescriptionTest, ProductDescriptionDatabaseTestFixture, Values(
    ProductDescription({ .name = "prod1", .barcode = "12345", .daysValidSuggestion = 3, .imagePath = "path/to/image1", .isArchived = true}),
    ProductDescription({ .name = "prod2", .barcode = "23456", .daysValidSuggestion = 2, .imagePath = "path/to/image2", .isArchived = false}),
    ProductDescription({ .name = "prod3", .barcode = "11345", .daysValidSuggestion = 3, .imagePath = std::nullopt, .isArchived = true}),
    ProductDescription({ .name = "prod4", .barcode = "22456", .daysValidSuggestion = 4, .imagePath = std::nullopt, .isArchived = false}),
    ProductDescription({ .name = "prod5", .barcode = std::nullopt, .daysValidSuggestion = 1, .imagePath = "path/to/image5", .isArchived = true}),
    ProductDescription({ .name = "prod6", .barcode = std::nullopt, .daysValidSuggestion = 2, .imagePath = "path/to/image6", .isArchived = false}),
    ProductDescription({ .name = "prod7", .barcode = std::nullopt, .daysValidSuggestion = 1, .imagePath = std::nullopt, .isArchived = true}),
    ProductDescription({ .name = "prod8", .barcode = std::nullopt, .daysValidSuggestion = 0, .imagePath = std::nullopt, .isArchived = false})
));

/* Tests for ProductInstance entities management */

struct ProductInstanceDatabaseTestFixture : DatabaseTestFixture, public WithParamInterface<ProductInstance>
{
};

TEST_P(ProductInstanceDatabaseTestFixture, DatabaseShouldCreateProductInstanceAccordingToCreateMethodParameters)
{
    auto templInst = GetParam();
    auto description = db.create<ProductDescription>();
    templInst.setFkId(description->getId());
    templInst.description = description;
    auto newInst = db.create<ProductInstance>(
        description, templInst.purchaseDate, templInst.expirationDate,
        templInst.daysToExpireWhenOpened, templInst.isOpen, templInst.isConsumed
    );
    assertProductInstancesAreEqual(templInst, *newInst);
}

INSTANTIATE_TEST_SUITE_P(ProductInstanceTest, ProductInstanceDatabaseTestFixture, Values(
    ProductInstance({
        .purchaseDate = parseIsoDate("2024-11-17"), .expirationDate = parseIsoDate("2024-12-17"),
        .daysToExpireWhenOpened = 3, .isOpen = true, .isConsumed = true }),
    ProductInstance({
        .purchaseDate = parseIsoDate("2024-11-17"), .expirationDate = parseIsoDate("2024-12-17"),
        .daysToExpireWhenOpened = 3, .isOpen = true, .isConsumed = false })
));

}