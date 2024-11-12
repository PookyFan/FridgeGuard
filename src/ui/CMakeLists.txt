cmake_minimum_required(VERSION 3.28.0)

find_package(Qt5 COMPONENTS REQUIRED Core Gui Widgets)

set(CMAKE_AUTOMOC ON)

file(GLOB UiSrc "*.cpp")
qt5_wrap_cpp(UiSrc "include/MainWindow.hpp")
add_library(UiLib STATIC ${UiSrc})
target_include_directories(UiLib PUBLIC "include")
target_link_libraries(UiLib PUBLIC Qt5::Core Qt5::Gui Qt5::Widgets)