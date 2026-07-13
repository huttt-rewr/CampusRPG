# CMakeLists.txt
# Qt Widgets 图形界面版构建脚本。
cmake_minimum_required(VERSION 3.16)
project(CampusRPG LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/release)

find_package(Qt5 COMPONENTS Widgets REQUIRED)

add_executable(CampusRPG_GUI
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/Profession.cpp
    include/Profession.h
)

target_include_directories(CampusRPG_GUI PRIVATE include src)
target_link_libraries(CampusRPG_GUI PRIVATE Qt5::Widgets)
