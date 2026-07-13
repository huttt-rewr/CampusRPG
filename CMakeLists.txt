# CMakeLists.txt
# 标准 C++ 控制台版构建脚本，不依赖 Qt 或第三方库。
cmake_minimum_required(VERSION 3.16)
project(CampusRPG LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/release)

include_directories(include)

add_executable(CampusRPG
    src/main.cpp
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
