cmake_minimum_required(VERSION 3.16)
project(CampusRPG_GUI VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt6 first, fall back to Qt5
find_package(Qt6 COMPONENTS Widgets QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 5.15 REQUIRED COMPONENTS Widgets)
    set(QT_LIBS Qt5::Widgets)
    message(STATUS "Using Qt5")
else()
    set(QT_LIBS Qt6::Widgets)
    message(STATUS "Using Qt6")
endif()

set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/MainWindow.h
    src/Character.cpp
    src/Item.cpp
    src/Food.cpp
    src/Medicine.cpp
    src/Equipment.cpp
    src/Enemy.cpp
    src/Task.cpp
    src/Shop.cpp
    src/GameManager.cpp
)

set(HEADERS
    include/Character.h
    include/Item.h
    include/Food.h
    include/Medicine.h
    include/Equipment.h
    include/Enemy.h
    include/Task.h
    include/Shop.h
    include/GameManager.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE include src)
target_link_libraries(${PROJECT_NAME} PRIVATE ${QT_LIBS})

# Compiler warnings
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /utf-8)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)
endif()
