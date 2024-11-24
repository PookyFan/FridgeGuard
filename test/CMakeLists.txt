cmake_minimum_required(VERSION 3.28.0)

file(GLOB TestSrc "./*.cpp")
add_executable(UnitTestsExec ${TestSrc})
target_link_libraries(UnitTestsExec DbLib gtest gtest_main gmock)
gtest_add_tests(TARGET UnitTestsExec)