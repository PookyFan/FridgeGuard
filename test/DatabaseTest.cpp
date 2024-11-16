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

    Database db{};
};

struct ProductCategoryDatabaseTestFixture : DatabaseTestFixture, public WithParamInterface<ProductCategory>
{};

TEST_P(ProductCategoryDatabaseTestFixture, DatabaseShouldCreateProductCategoryAccordingToCreateMethodParameters)
{
    auto templCat = GetParam();
    EntityPtr<ProductCategory> newCat = db.create<ProductCategory>(templCat.name, templCat.imagePath, templCat.isArchived);
    assertProductCategoriesAreEqual(templCat, *newCat);
}

INSTANTIATE_TEST_SUITE_P(ProductCategoryTest, ProductCategoryDatabaseTestFixture, Values(
    ProductCategory{ .name = "cat1", .imagePath = "path/to/image1", .isArchived = true },
    ProductCategory{ .name = "cat2", .imagePath = "path/to/image2", .isArchived = false },
    ProductCategory{ .name = "cat3", .imagePath = std::nullopt, .isArchived = true },
    ProductCategory{ .name = "cat4", .imagePath = std::nullopt, .isArchived = false }
));

template<typename T>
struct TypedDatabaseTestFixture : DatabaseTestFixture
{};

using EntitiesTypes = Types<ProductCategory, ProductDescription, ProductInstance>;
TYPED_TEST_SUITE(TypedDatabaseTestFixture, EntitiesTypes);

TYPED_TEST(TypedDatabaseTestFixture, DatabaseShouldCreateEntitiesWithIncreasingIds)
{
    constexpr auto numOfEntities = 100;
    for(auto i = 1; i <= numOfEntities; ++i)
        ASSERT_EQ(i, this->db.template create<TypeParam>()->id);
}

}