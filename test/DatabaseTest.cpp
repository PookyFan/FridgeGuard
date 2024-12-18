#include <vector>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "Database.hpp"

using namespace testing;

namespace FG::data::test
{
namespace
{
constexpr Id exampleId = 3;
constexpr Id exampleFkId = 5;
constexpr char noEntityInDbErrStr[] = "No entity in DB";

struct TestEmptySchema {};
using TestEmptyEntity = DbEntity<TestEmptySchema>;

struct TestSimpleSchema
{
    int number;
    std::string label;
};
using TestSimpleEntity = DbEntity<TestSimpleSchema>;

struct TestComplexSchema
{
    using FkEntity = TestSimpleEntity;

    Nullable<int> optNumber;
    Nullable<bool> optFlag;
};

struct TestComplexEntity : public DbEntity<TestComplexSchema>
{
    using Base = DbEntity<TestComplexSchema>;

    TestComplexEntity(TestComplexSchema&& data) : Base(std::move(data))
    {}

    TestComplexEntity(EntityPtr<TestSimpleEntity> fkEnt, TestComplexSchema&& data)
        : Base(fkEnt, std::move(data)), simpleEntity(fkEnt)
    {}

    void setFkEntity(EntityPtr<const TestSimpleEntity> newFkEntity)
    {
        simpleEntity = newFkEntity;
        setFkId(newFkEntity->getId());
    }

    EntityPtr<const TestSimpleEntity> simpleEntity;
};

template<typename T>
struct TypeInd {};

template<typename T>
struct FilterTypeInd {};

class TestDatabase : public Database<TestDatabase, TestEmptyEntity, TestSimpleEntity, TestComplexEntity>
{
public:
    MOCK_METHOD(void, insertMock, (TestEmptyEntity&), ());
    MOCK_METHOD(void, insertMock, (TestSimpleEntity&), ());
    MOCK_METHOD(void, insertMock, (TestComplexEntity&), ());

    MOCK_METHOD(TestEmptyEntity, retrieveSingleMock, (Id, TypeInd<TestEmptyEntity>), ());
    MOCK_METHOD(TestSimpleEntity, retrieveSingleMock, (Id, TypeInd<TestSimpleEntity>), ());
    MOCK_METHOD(TestComplexEntity, retrieveSingleMock, (Id, TypeInd<TestComplexEntity>), ());

    MOCK_METHOD(std::vector<TestEmptyEntity>, retrieveMultipleMock, (const std::set<Id>&, TypeInd<TestEmptyEntity>), ());
    MOCK_METHOD(std::vector<TestSimpleEntity>, retrieveMultipleMock, (const std::set<Id>&, TypeInd<TestSimpleEntity>), ());
    MOCK_METHOD(std::vector<TestComplexEntity>, retrieveMultipleMock, (const std::set<Id>&, TypeInd<TestComplexEntity>), ());

    MOCK_METHOD(std::vector<TestEmptyEntity>, retrieveFilteredMock, (FilterTypeInd<TestEmptyEntity>), ());
    MOCK_METHOD(std::vector<TestSimpleEntity>, retrieveFilteredMock, (FilterTypeInd<TestSimpleEntity>), ());
    MOCK_METHOD(std::vector<TestComplexEntity>, retrieveFilteredMock, (FilterTypeInd<TestComplexEntity>), ());

    MOCK_METHOD(void, updateMock, (const TestEmptyEntity&), ());
    MOCK_METHOD(void, updateMock, (const TestSimpleEntity&), ());
    MOCK_METHOD(void, updateMock, (const TestComplexEntity&), ());

    MOCK_METHOD(void, removeMock, (TestEmptyEntity&), ());
    MOCK_METHOD(void, removeMock, (TestSimpleEntity&), ());
    MOCK_METHOD(void, removeMock, (TestComplexEntity&), ());

    template<typename EntityT>
    void insertImpl(EntityT& entity)
    {
        entity.setId(++nextId);
        insertMock(entity);
    }

    template<typename EntityT>
    EntityT retrieveImpl(Id id)
    {
        return retrieveSingleMock(id, TypeInd<EntityT>());
    }

    template<typename EntityT>
    std::vector<EntityT> retrieveImpl(const std::set<Id>& ids)
    {
        return retrieveMultipleMock(ids, TypeInd<EntityT>());
    }

    template<typename EntityT>
    std::vector<EntityT> retrieveImpl(FilterTypeInd<EntityT> filter)
    {
        return retrieveFilteredMock(filter);
    }

    template<typename EntityT>
    void updateImpl(const EntityT& entity)
    {
        updateMock(entity);
    }

    template<typename EntityT>
    void removeImpl(EntityT& entity)
    {
        removeMock(entity);
    }

private:
    Id nextId = 0;
};
}

struct DatabaseTestFixture : public Test
{
    template<typename EntityT>
    void expectSingleRetrieveById(Id entityId, const EntityT& entityTemplate)
    {
        EXPECT_CALL(db, retrieveSingleMock(entityId, An<TypeInd<EntityT>>())).WillOnce(Return(entityTemplate)).RetiresOnSaturation();
    }

    StrictMock<TestDatabase> db;
};

TEST_F(DatabaseTestFixture, DatabaseShouldReturnComplexEntityWithFkThatWasNotExplicitlyCreatedButIsPresentInUnderlyingDb)
{
    TestSimpleEntity fkEntityTemplate({});
    TestComplexEntity entityTemplate({});
    fkEntityTemplate.setId(exampleFkId);
    entityTemplate.setId(exampleId);
    entityTemplate.setFkId(exampleFkId);
    expectSingleRetrieveById(exampleId, entityTemplate);
    expectSingleRetrieveById(exampleFkId, fkEntityTemplate);

    auto entityPtr = db.template retrieve<TestComplexEntity>(exampleId);
    ASSERT_EQ(entityTemplate.getId(), entityPtr->getId());
    ASSERT_EQ(entityTemplate.getFkId(), entityPtr->getFkId());
    ASSERT_EQ(fkEntityTemplate.getId(), entityPtr->simpleEntity->getId());

    auto theSameEntityPtr = db.template retrieve<TestComplexEntity>(entityPtr->getId());
    ASSERT_EQ(entityPtr.get(), theSameEntityPtr.get());
    ASSERT_EQ(entityPtr->simpleEntity.get(), theSameEntityPtr->simpleEntity.get());
}

TEST_F(DatabaseTestFixture, DatabaseShouldReturnEntitiesFromCacheAsLongAsThereAreEntityPtrObjectsKeepingReferencesToThem)
{
    TestSimpleEntity fkEntityTemplate({});
    TestComplexEntity entityTemplate({});
    Id entityId;
    Id fkEntityId;

    {
    EXPECT_CALL(db, insertMock(An<TestSimpleEntity&>()));
    EXPECT_CALL(db, insertMock(An<TestComplexEntity&>()));
    auto fkEntityPtr = db.template create<TestSimpleEntity>();
    auto entityPtr = db.template create<TestComplexEntity>(fkEntityPtr);
    entityTemplate = *entityPtr.get();
    fkEntityTemplate = *fkEntityPtr.get();
    entityTemplate.simpleEntity.reset();
    entityId = entityTemplate.getId();
    fkEntityId = fkEntityTemplate.getId();
    ASSERT_EQ(fkEntityPtr.get(), entityPtr->simpleEntity.get());

    auto secondEntityPtr = db.template retrieve<TestComplexEntity>(entityId);
    auto secondFkEntityPtr = db.template retrieve<TestSimpleEntity>(fkEntityId);
    ASSERT_EQ(entityPtr.get(), secondEntityPtr.get());
    ASSERT_EQ(fkEntityPtr.get(), secondFkEntityPtr.get());
    }

    {
    expectSingleRetrieveById(entityId, entityTemplate);
    expectSingleRetrieveById(fkEntityId, fkEntityTemplate);

    auto entityPtr = db.template retrieve<TestComplexEntity>(entityId);
    auto fkEntityPtr = db.template retrieve<TestSimpleEntity>(fkEntityId);
    auto secondEntityPtr = db.template retrieve<TestComplexEntity>(entityId);
    auto secondFkEntityPtr = db.template retrieve<TestSimpleEntity>(fkEntityId);
    }

    {
    expectSingleRetrieveById(entityId, entityTemplate);
    expectSingleRetrieveById(fkEntityId, fkEntityTemplate);

    auto entityPtr = db.template retrieve<TestComplexEntity>(entityId);
    auto secondEntityPtr = db.template retrieve<TestComplexEntity>(entityId);
    auto thirdEntityPtr = db.template retrieve<TestComplexEntity>(entityId);
    }

    {
    std::vector<TestSimpleEntity> expectedFkEntities {fkEntityTemplate};
    std::vector<TestComplexEntity> expectedEntities {entityTemplate};
    EXPECT_CALL(this->db, retrieveFilteredMock(An<FilterTypeInd<TestComplexEntity>>())).WillOnce(Return(expectedEntities));
    EXPECT_CALL(this->db, retrieveMultipleMock(An<const std::set<Id>&>(), An<TypeInd<TestSimpleEntity>>())).WillOnce(Return(expectedFkEntities));

    auto entitiesPtrs = this->db.template retrieve<TestComplexEntity>(FilterTypeInd<TestComplexEntity>());
    auto entityPtr = db.template retrieve<TestComplexEntity>(entityId);
    auto fkEntityPtr = db.template retrieve<TestSimpleEntity>(fkEntityId);
    }

    {
    expectSingleRetrieveById(fkEntityId, fkEntityTemplate);

    auto fkEntityPtr = db.template retrieve<TestSimpleEntity>(fkEntityId);
    auto secondFkEntityPtr = db.template retrieve<TestSimpleEntity>(fkEntityId);
    }
}

TEST_F(DatabaseTestFixture, DatabaseShouldNotOverwriteCachedEntitiesWhenTheyAreRetrievedAgainWithFilterFromUnderlyingDb)
{
    TestSimpleEntity entityTemplate({3, "test"});
    entityTemplate.setId(exampleId);
    expectSingleRetrieveById(exampleId, entityTemplate);

    auto entityPtr = db.template retrieve<TestSimpleEntity>(exampleId);
    ASSERT_EQ(entityTemplate.getId(), entityPtr->getId());
    ASSERT_EQ(entityTemplate.number, entityPtr->number);
    ASSERT_EQ(entityTemplate.label, entityPtr->label);

    entityPtr->number = 5;
    entityPtr->label = "other test";
    ASSERT_EQ(entityTemplate.getId(), entityPtr->getId());
    ASSERT_NE(entityTemplate.number, entityPtr->number);
    ASSERT_NE(entityTemplate.label, entityPtr->label);

    auto secondEntityPtr = db.template retrieve<TestSimpleEntity>(exampleId);
    ASSERT_EQ(entityPtr.get(), secondEntityPtr.get());

    std::vector<TestSimpleEntity> expectedEntities {entityTemplate};
    EXPECT_CALL(this->db, retrieveFilteredMock(An<FilterTypeInd<TestSimpleEntity>>())).WillOnce(Return(expectedEntities));

    auto entitiesPtrs = this->db.template retrieve<TestSimpleEntity>(FilterTypeInd<TestSimpleEntity>());
    ASSERT_EQ(1, entitiesPtrs.size());
    ASSERT_EQ(entitiesPtrs.front().get(), entityPtr.get());
    ASSERT_EQ(entityTemplate.getId(), entitiesPtrs.front()->getId());
    ASSERT_NE(entityTemplate.number, entitiesPtrs.front()->number);
    ASSERT_NE(entityTemplate.label, entitiesPtrs.front()->label);
}

TEST_F(DatabaseTestFixture, DatabaseShouldThrowWhenTryingToGetComplexEntityWithFkEntityThatWasNotExplicitlyCreatedAndIsNotPresentInUnderlyingDb)
{
    EXPECT_CALL(db, retrieveSingleMock(exampleId, An<TypeInd<TestComplexEntity>>())).WillOnce(Throw(std::runtime_error("No entity in DB")));
    ASSERT_THROW(db.template retrieve<TestComplexEntity>(exampleId), std::runtime_error);
}

template<typename T>
struct TypedDatabaseTestFixture : DatabaseTestFixture
{};

using NonFkEntitiesTypes = Types<TestEmptyEntity, TestSimpleEntity>;
TYPED_TEST_SUITE(TypedDatabaseTestFixture, NonFkEntitiesTypes);

TYPED_TEST(TypedDatabaseTestFixture, EntityShouldThrowWhenAttemptingToChangeItsIdAfterCreationByDatabase)
{
    EXPECT_CALL(this->db, insertMock(An<TypeParam&>()));

    auto entity = this->db.template create<TypeParam>();
    ASSERT_THROW(entity->setId(2), std::runtime_error);
}

TYPED_TEST(TypedDatabaseTestFixture, DatabaseShouldReturnEntityThatWasJustCreatedFromEntityCacheWithoutQueryingUnderlyingDb)
{
    EXPECT_CALL(this->db, insertMock(An<TypeParam&>()));

    auto entityPtr = this->db.template create<TypeParam>();
    auto theSameEntityPtr = this->db.template retrieve<TypeParam>(entityPtr->getId());
    ASSERT_EQ(entityPtr.get(), theSameEntityPtr.get());
}

TYPED_TEST(TypedDatabaseTestFixture, DatabaseShouldReturnEntityThatWasNotExplicitlyCreatedButIsPresentInUnderlyingDb)
{
    TypeParam entityTemplate({});
    entityTemplate.setId(exampleId);
    this->expectSingleRetrieveById(exampleId, entityTemplate);

    auto entityPtr = this->db.template retrieve<TypeParam>(exampleId);
    ASSERT_EQ(entityTemplate.getId(), entityPtr->getId());

    auto theSameEntityPtr = this->db.template retrieve<TypeParam>(entityPtr->getId());
    ASSERT_EQ(entityPtr.get(), theSameEntityPtr.get());
}

TYPED_TEST(TypedDatabaseTestFixture, DatabaseShouldThrowWhenTryingToGetEntityThatWasNotExplicitlyCreatedAndIsNotPresentInUnderlyingDb)
{
    EXPECT_CALL(this->db, retrieveSingleMock(exampleId, An<TypeInd<TypeParam>>())).WillOnce(Throw(std::runtime_error(noEntityInDbErrStr)));
    ASSERT_THROW(this->db.template retrieve<TypeParam>(exampleId), std::runtime_error);
}

TYPED_TEST(TypedDatabaseTestFixture, DatabaseShouldCacheEntitiesRetrievedWithFiltersFromUnderlyingDbAsLongAsThereAreEntityPtrObjectsKeepingReferencesToThem)
{
    constexpr auto numOfEntities = 10;
    constexpr auto firstEntityId = 5;
    TypeParam dummyEntity({});
    dummyEntity.setId(exampleId);
    std::vector<TypeParam> expectedEntities(numOfEntities);
    for(auto i = 0; i < numOfEntities; ++i)
        expectedEntities[i].setId(firstEntityId + i);
    EXPECT_CALL(this->db, retrieveFilteredMock(An<FilterTypeInd<TypeParam>>())).WillOnce(Return(expectedEntities));
    EXPECT_CALL(this->db, retrieveSingleMock(Lt(firstEntityId), An<TypeInd<TypeParam>>())).WillRepeatedly(Return(dummyEntity));
    EXPECT_CALL(this->db, retrieveSingleMock(Gt(firstEntityId), An<TypeInd<TypeParam>>())).WillRepeatedly(Return(dummyEntity));

    {
    auto entitiesPtrs = this->db.template retrieve<TypeParam>(FilterTypeInd<TypeParam>());
    for(auto i = 1; i < firstEntityId; ++i)
        ASSERT_EQ(dummyEntity.getId(), this->db.template retrieve<TypeParam>(i)->getId());
    for(auto i = firstEntityId; i < firstEntityId + numOfEntities; ++i)
        ASSERT_EQ(i, this->db.template retrieve<TypeParam>(i)->getId());
    for(auto i = firstEntityId + numOfEntities; i < firstEntityId + 2*numOfEntities; ++i)
        ASSERT_EQ(dummyEntity.getId(), this->db.template retrieve<TypeParam>(i)->getId());
    }

    this->expectSingleRetrieveById(firstEntityId, dummyEntity);
    for(auto i = 1; i < firstEntityId + 2*numOfEntities; ++i)
        ASSERT_EQ(dummyEntity.getId(), this->db.template retrieve<TypeParam>(i)->getId());
}

TYPED_TEST(TypedDatabaseTestFixture, DatabaseShouldAllowForCachingManyEntitiesWithoutFailure)
{
    constexpr auto numOfEntities = 1'000'000;
    std::vector<EntityPtr<TypeParam>> entities;
    entities.reserve(numOfEntities);
    EXPECT_CALL(this->db, insertMock(An<TypeParam&>())).Times(numOfEntities);

    for(auto i = numOfEntities; i > 0; --i)
        entities.push_back(this->db.template create<TypeParam>());
    for(auto i = 1; i <= numOfEntities; ++i)
        ASSERT_EQ(i, entities[i - 1]->getId());
}

TYPED_TEST(TypedDatabaseTestFixture, DatabaseShouldForwardEntityUpdateRequestToUnderlyingDb)
{
    EXPECT_CALL(this->db, insertMock(An<TypeParam&>()));
    EXPECT_CALL(this->db, updateMock(An<const TypeParam&>()));

    auto entityPtr = this->db.template create<TypeParam>();
    this->db.commitChanges(entityPtr);
}

TYPED_TEST(TypedDatabaseTestFixture, DatabaseShouldInvalidateEntityPtrUponDeletionAndNotReturnDeletedEntityFromCacheAnymore)
{
    EXPECT_CALL(this->db, insertMock(An<TypeParam&>()));
    EXPECT_CALL(this->db, removeMock(An<TypeParam&>()));

    auto entityPtr = this->db.template create<TypeParam>();
    const auto entityId = entityPtr->getId();
    auto secondEntityPtr = this->db.template retrieve<TypeParam>(entityId);
    ASSERT_TRUE(entityPtr);
    ASSERT_TRUE(entityPtr->isValid());
    ASSERT_TRUE(secondEntityPtr);
    ASSERT_TRUE(secondEntityPtr->isValid());
    
    this->db.template remove<TypeParam>(std::move(entityPtr));
    ASSERT_FALSE(entityPtr);
    ASSERT_TRUE(secondEntityPtr);
    ASSERT_FALSE(secondEntityPtr->isValid());

    EXPECT_CALL(this->db, retrieveSingleMock(entityId, An<TypeInd<TypeParam>>())).WillOnce(Throw(std::runtime_error(noEntityInDbErrStr)));
    ASSERT_THROW(this->db.template retrieve<TypeParam>(entityId), std::runtime_error);
}
}