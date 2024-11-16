#include <vector>

#include <gtest/gtest.h>
#include "Database.hpp"

using namespace testing;

namespace FG::data::test
{
// template<typename T>
// using EntityVector = std::vector<EntityPtr<T>>;

struct DatabaseTestFixture : public Test
{
    Database db;
};

template<typename T>
struct TypedDatabaseTestFixture : public DatabaseTestFixture
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