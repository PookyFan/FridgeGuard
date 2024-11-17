cmake_minimum_required(VERSION 3.28.0)
project(FridgeGuard VERSION 0.1.0 LANGUAGES CXX)

set(OUTPUT_BIN_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_BIN_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_BIN_DIRECTORY})

enable_testing()

add_subdirectory(dependencies)
add_subdirectory(src)
add_subdirectory(test)