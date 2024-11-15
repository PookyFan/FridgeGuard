cmake_minimum_required(VERSION 3.28.0)

include(FetchContent)

#sqlite_orm
FetchContent_Declare(
  sqliteOrm
  GIT_REPOSITORY https://github.com/fnc12/sqlite_orm
  GIT_TAG        v1.9
)

add_subdirectory(sqlite_orm)

#googletest
find_package(GTest)
if(NOT GTest_FOUND)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.15.2
    )
    FetchContent_MakeAvailable(googletest)
endif()