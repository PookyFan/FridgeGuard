cmake_minimum_required(VERSION 3.28.0)

file(GLOB DbSrc "*.cpp")
add_library(DbLib STATIC ${DbSrc})
target_include_directories(DbLib PUBLIC "include")
target_link_libraries(DbLib PRIVATE sqlite_orm::sqlite_orm)