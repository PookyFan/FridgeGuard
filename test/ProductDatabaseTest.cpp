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
    //Part 1: create entities and retrieve them by IDs
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
            templInst.daysToExpireWhenOpened, templInst.isOpen, templInst.isConsumed);
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
    //End of part 1

    //Part 2: retrieve instances by set of IDs
    {
    const std::set<Id> ids {5, 6};
    auto filteredInstances = db.retrieve<ProductInstance>(ids);
    ASSERT_EQ(2, filteredInstances.size());
    for(Id ii : ids)
    {
        Id fi = ii - *ids.begin();
        Id di = --ii % sampleProductDescriptions.size();
        Id ci = ii % sampleProductCategories.size();
        assertProductInstancesAreEqual(sampleProductInstances[ii], *filteredInstances[fi]);
        assertProductDescriptionsAreEqual(sampleProductDescriptions[di], *filteredInstances[fi]->description);
        assertProductCategoriesAreEqual(sampleProductCategories[ci], *filteredInstances[fi]->description->category);
    }
    }
    //End of part 2

    //Part 3: retrieve filtered instanced
    {
    using namespace sqlite_orm;
    auto filteredInstances = db.retrieve<ProductInstance>(
        where(
            c(column<ProductInstance>(&ProductInstance::getExpirationDateTimestamp)) > isoDateToTimestamp("2024-12-01")
            and
            is_not_null(column<ProductInstance>(&ProductInstance::daysToExpireWhenOpened)))
    );
    ASSERT_EQ(3, filteredInstances.size());
    assertProductInstancesAreEqual(sampleProductInstances[0], *filteredInstances[0]);
    assertProductDescriptionsAreEqual(sampleProductDescriptions[0], *filteredInstances[0]->description);
    assertProductCategoriesAreEqual(sampleProductCategories[0], *filteredInstances[0]->description->category);

    assertProductInstancesAreEqual(sampleProductInstances[1], *filteredInstances[1]);
    assertProductDescriptionsAreEqual(sampleProductDescriptions[1], *filteredInstances[1]->description);
    assertProductCategoriesAreEqual(sampleProductCategories[1], *filteredInstances[1]->description->category);

    assertProductInstancesAreEqual(sampleProductInstances[3], *filteredInstances[2]);
    assertProductDescriptionsAreEqual(sampleProductDescriptions[3], *filteredInstances[2]->description);
    assertProductCategoriesAreEqual(sampleProductCategories[1], *filteredInstances[2]->description->category);
    }
    //End of part 3

    //Part 4: retrieve all entities
    {
    auto allCategories = db.retrieveAll<ProductCategory>();
    ASSERT_EQ(sampleProductCategories.size(), allCategories.size());
    for(auto i = 0; i < sampleProductCategories.size(); ++i)
        assertProductCategoriesAreEqual(sampleProductCategories[i], *allCategories[i]);
    
    auto allDescriptions = db.retrieveAll<ProductDescription>();
    auto categoriesIt = sampleProductCategories.begin();
    ASSERT_EQ(sampleProductDescriptions.size(), allDescriptions.size());
    for(auto i = 0; i < sampleProductCategories.size(); ++i)
    {
        if(categoriesIt == sampleProductCategories.end())
            categoriesIt = sampleProductCategories.begin();
        assertProductDescriptionsAreEqual(sampleProductDescriptions[i], *allDescriptions[i]);
        assertProductCategoriesAreEqual(*(categoriesIt++), *allDescriptions[i]->category);
    }
    
    auto allInstances = db.retrieveAll<ProductInstance>();
    auto descriptionsIt = sampleProductDescriptions.begin();
    categoriesIt = sampleProductCategories.begin();
    ASSERT_EQ(sampleProductInstances.size(), allInstances.size());
    for(auto i = 0; i < sampleProductInstances.size(); ++i)
    {
        if(categoriesIt == sampleProductCategories.end())
            categoriesIt = sampleProductCategories.begin();
        if(descriptionsIt == sampleProductDescriptions.end())
            descriptionsIt = sampleProductDescriptions.begin();
        assertProductInstancesAreEqual(sampleProductInstances[i], *allInstances[i]);
        assertProductDescriptionsAreEqual(*(descriptionsIt++), *allInstances[i]->description);
        assertProductCategoriesAreEqual(*(categoriesIt++), *allInstances[i]->description->category);
    }
    }
    //End of part 4
}

TEST_F(ProductDatabaseTestFixture, ProductDatabaseShouldUpdateEntitiesBothInCacheAndInDbStorage)
{
    auto firstTemplCat = sampleProductCategories.front();
    auto secondTemplCat = sampleProductCategories.back();
    auto firstTemplDesc = sampleProductDescriptions.front();
    auto secondTemplDesc = sampleProductDescriptions.back();
    auto templInst = sampleProductInstances.front();

    {
    auto firstCat = db.create<ProductCategory>(firstTemplCat.name, firstTemplCat.imagePath, firstTemplCat.isArchived);
    auto secondCat = db.create<ProductCategory>(secondTemplCat.name, secondTemplCat.imagePath, secondTemplCat.isArchived);

    auto firstDesc = db.create<ProductDescription>(
            firstCat, firstTemplDesc.name, firstTemplDesc.barcode,
            firstTemplDesc.daysValidSuggestion,
            firstTemplDesc.imagePath, firstTemplDesc.isArchived);
    auto secondDesc = db.create<ProductDescription>(
            secondCat, secondTemplDesc.name, secondTemplDesc.barcode,
            secondTemplDesc.daysValidSuggestion,
            secondTemplDesc.imagePath, secondTemplDesc.isArchived);
    
    auto instance = db.create<ProductInstance>(
            firstDesc, templInst.purchaseDate, templInst.expirationDate,
            templInst.daysToExpireWhenOpened, templInst.isOpen, templInst.isConsumed);
    
    instance->expirationDate = parseIsoDate("2025-01-31");
    instance->daysToExpireWhenOpened = 5;
    instance->isConsumed = false;
    db.commitChanges(instance);
    templInst = *instance;
    assertProductInstancesAreEqual(templInst, *instance);

    firstCat->name = "otherName";
    firstCat->imagePath = std::nullopt;
    db.commitChanges(firstCat);
    firstTemplCat = *firstCat;
    assertProductCategoriesAreEqual(firstTemplCat, *firstCat);

    auto secondInstance =  db.retrieve<ProductInstance>(1);
    assertProductInstancesAreEqual(templInst, *secondInstance);
    }

    {
    auto instance =  db.retrieve<ProductInstance>(1);
    assertProductInstancesAreEqual(templInst, *instance);
    assertProductCategoriesAreEqual(firstTemplCat, *instance->description->category);

    auto secondDesc = db.retrieve<ProductDescription>(2);
    instance->description = secondDesc;
    db.commitChanges(instance);
    assertProductDescriptionsAreEqual(secondTemplDesc, *instance->description);
    assertProductCategoriesAreEqual(secondTemplCat, *instance->description->category);

    auto secondInstance =  db.retrieve<ProductInstance>(1);
    assertProductDescriptionsAreEqual(secondTemplDesc, *secondInstance->description);
    assertProductCategoriesAreEqual(secondTemplCat, *secondInstance->description->category);

    secondDesc->name = "someOtherName";
    secondDesc->barcode = "777777";
    secondDesc->daysValidSuggestion = 4;
    db.commitChanges(secondDesc);
    secondTemplDesc = *secondDesc;

    assertProductDescriptionsAreEqual(secondTemplDesc, *secondDesc);
    }

    auto instance =  db.retrieve<ProductInstance>(1);
    assertProductInstancesAreEqual(templInst, *instance);
    assertProductDescriptionsAreEqual(secondTemplDesc, *instance->description);
    assertProductCategoriesAreEqual(secondTemplCat, *instance->description->category);
}

/* Generic entities management tests */

template<typename T>
struct TypedProductDatabaseTestFixture : ProductDatabaseTestFixture
{
    template<WithFkEntity EntityT>
    auto createEntityInDatabase()
    {
        auto fkEntityPtr = createEntityInDatabase<typename EntityT::FkEntity>();
        return db.template create<EntityT>(fkEntityPtr);
    }

    template<typename EntityT>
    auto createEntityInDatabase()
    {
        return db.template create<EntityT>();
    }
};

using EntitiesTypes = Types<ProductCategory, ProductDescription, ProductInstance>;
TYPED_TEST_SUITE(TypedProductDatabaseTestFixture, EntitiesTypes);

TYPED_TEST(TypedProductDatabaseTestFixture, ProductDatabaseShouldCreateEntitiesWithIncreasingIds)
{
    constexpr auto numOfEntities = 100;
    for(auto i = 1; i <= numOfEntities; ++i)
        ASSERT_EQ(i, this->template createEntityInDatabase<TypeParam>()->getId());
}

TYPED_TEST(TypedProductDatabaseTestFixture, ProductDatabaseShouldDeleteEntitiesFromUnderlyingStorageAndNotReturnThemFromCache)
{
    constexpr auto numOfEntities = 10;
    std::array<EntityPtr<TypeParam>, numOfEntities> entities;
    for(auto i = 0; i < numOfEntities; ++i)
        entities[i] = this->template createEntityInDatabase<TypeParam>();

    constexpr auto removedEntityId = numOfEntities / 2;
    auto& removedEntity = entities[removedEntityId - 1];
    ASSERT_TRUE(removedEntity);
    ASSERT_TRUE(removedEntity->isValid());

    this->db.template remove<TypeParam>(std::move(removedEntity));
    ASSERT_FALSE(removedEntity);
    ASSERT_THROW(this->db.template retrieve<TypeParam>(removedEntityId), std::system_error);

    for(auto i = 1; i <= numOfEntities; ++i)
    {
        if(i == removedEntityId)
            continue;
        ASSERT_NO_THROW(this->db.template retrieve<TypeParam>(i));
    }
}

}