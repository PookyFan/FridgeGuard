cmake_minimum_required(VERSION 3.28.0)

add_subdirectory(ui)
add_subdirectory(data)

add_executable(FridgeGuardApp main.cpp)
target_link_libraries(FridgeGuardApp UiLib DbLib)