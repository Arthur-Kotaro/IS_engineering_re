cmake_minimum_required(VERSION 3.16)
project(CorporateClient VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Опции сборки
option(BUILD_ENGINEERING_CLIENT "Build engineering console client" ON)
option(BUILD_PRODUCTION_CLIENT "Build production GUI client" ON)
option(BUILD_TESTS "Build unit and integration tests" OFF)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS Core Network)

if(BUILD_PRODUCTION_CLIENT)
    find_package(Qt6 REQUIRED COMPONENTS Qml Quick QuickControls2)
endif()

# Общая библиотека
add_subdirectory(common)

# Клиенты
if(BUILD_ENGINEERING_CLIENT)
    add_subdirectory(engineering_client)
endif()

if(BUILD_PRODUCTION_CLIENT)
    add_subdirectory(production_client)
endif()
