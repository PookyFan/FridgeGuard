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

class TestDatabase : public Database<TestDatabase, TestEmptyEntity, TestSimpleEntity, TestComplexEntity>
{
public:
    MOCK_METHOD(void, insertMock, (TestEmptyEntity&), ());
    MOCK_METHOD(void, insertMock, (TestSimpleEntity&), ());
    MOCK_METHOD(void, insertMock, (TestComplexEntity&), ());

    MOCK_METHOD(TestEmptyEntity, retrieveMock, (Id, TypeInd<TestEmptyEntity>), ());
    MOCK_METHOD(TestSimpleEntity, retrieveMock, (Id, TypeInd<TestSimpleEntity>), ());
    MOCK_METHOD(TestComplexEntity, retrieveMock, (Id, TypeInd<TestComplexEntity>), ());

    template<typename EntityT>
    void insertImpl(EntityT& entity)
    {
        entity.setId(++nextId);
        insertMock(entity);
    }

    template<typename EntityT>
    EntityT retrieveImpl(Id id)
    {
        return retrieveMock(id, TypeInd<EntityT>());
    }

private:
    Id nextId = 0;
};
}

struct DatabaseTestFixture : public Test
{
    StrictMock<TestDatabase> db;
};

TEST_F(DatabaseTestFixture, DatabaseShouldReturnComplexEntityWithFkThatWasNotExplicitlyCreatedButIsPresentInUnderlyingDb)
{
    TestSimpleEntity fkEntityTemplate({});
    TestComplexEntity entityTemplate({});
    fkEntityTemplate.setId(exampleFkId);
    entityTemplate.setId(exampleId);
    entityTemplate.setFkId(exampleFkId);
    EXPECT_CALL(db, retrieveMock(exampleId, An<TypeInd<TestComplexEntity>>())).WillOnce(Return(entityTemplate));
    EXPECT_CALL(db, retrieveMock(exampleFkId, An<TypeInd<TestSimpleEntity>>())).WillOnce(Return(fkEntityTemplate));

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

    EXPECT_CALL(db, retrieveMock(entityId, An<TypeInd<TestComplexEntity>>())).Times(2).WillRepeatedly(Return(entityTemplate));
    EXPECT_CALL(db, retrieveMock(fkEntityId, An<TypeInd<TestSimpleEntity>>())).Times(3).WillRepeatedly(Return(fkEntityTemplate));

    {
        auto entityPtr = db.template retrieve<TestComplexEntity>(entityId);
        auto fkEntityPtr = db.template retrieve<TestSimpleEntity>(fkEntityId);
        auto secondEntityPtr = db.template retrieve<TestComplexEntity>(entityId);
        auto secondFkEntityPtr = db.template retrieve<TestSimpleEntity>(fkEntityId);
    }

    {
        auto entityPtr = db.template retrieve<TestComplexEntity>(entityId);
        auto secondEntityPtr = db.template retrieve<TestComplexEntity>(entityId);
        auto thirdEntityPtr = db.template retrieve<TestComplexEntity>(entityId);
    }

    {
        auto fkEntityPtr = db.template retrieve<TestSimpleEntity>(fkEntityId);
        auto secondFkEntityPtr = db.template retrieve<TestSimpleEntity>(fkEntityId);
    }
}

TEST_F(DatabaseTestFixture, DatabaseShouldThrowWhenTryingToGetComplexEntityWithFkEntityThatWasNotExplicitlyCreatedAndIsNotPresentInUnderlyingDb)
{
    EXPECT_CALL(db, retrieveMock(exampleId, An<TypeInd<TestComplexEntity>>())).WillOnce(Throw(std::runtime_error("No entity in DB")));
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
    EXPECT_CALL(this->db, retrieveMock(exampleId, An<TypeInd<TypeParam>>())).WillOnce(Return(entityTemplate));

    auto entityPtr = this->db.template retrieve<TypeParam>(exampleId);
    ASSERT_EQ(entityTemplate.getId(), entityPtr->getId());

    auto theSameEntityPtr = this->db.template retrieve<TypeParam>(entityPtr->getId());
    ASSERT_EQ(entityPtr.get(), theSameEntityPtr.get());
}

TYPED_TEST(TypedDatabaseTestFixture, DatabaseShouldThrowWhenTryingToGetEntityThatWasNotExplicitlyCreatedAndIsNotPresentInUnderlyingDb)
{
    EXPECT_CALL(this->db, retrieveMock(exampleId, An<TypeInd<TypeParam>>())).WillOnce(Throw(std::runtime_error("No entity in DB")));
    ASSERT_THROW(this->db.template retrieve<TypeParam>(exampleId), std::runtime_error);
}
}