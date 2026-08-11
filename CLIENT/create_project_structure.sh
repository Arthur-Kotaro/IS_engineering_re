#!/bin/bash

# Упрощённый скрипт создания структуры проекта
# Без цветов, без избыточных сообщений

# Функция создания директории
create_dir() {
    if [ ! -d "$1" ]; then
        mkdir -p "$1"
        echo "created: $1"
    else
        echo "exists: $1"
    fi
}

# Функция создания файла
create_file() {
    local file_path="$1"
    local content="$2"
    local dir_path=$(dirname "$file_path")
    
    if [ ! -d "$dir_path" ]; then
        mkdir -p "$dir_path"
        echo "created: $dir_path"
    fi
    
    if [ ! -f "$file_path" ]; then
        echo -e "$content" > "$file_path"
        echo "created: $file_path"
    else
        echo "exists: $file_path"
    fi
}

# Функция создания пустого файла
create_empty_file() {
    local file_path="$1"
    local dir_path=$(dirname "$file_path")
    
    if [ ! -d "$dir_path" ]; then
        mkdir -p "$dir_path"
        echo "created: $dir_path"
    fi
    
    if [ ! -f "$file_path" ]; then
        touch "$file_path"
        echo "created: $file_path"
    else
        echo "exists: $file_path"
    fi
}

echo "=== Creating project structure ==="

# Корневые файлы
create_file "CMakeLists.txt" "cmake_minimum_required(VERSION 3.16)
project(CorporateClient VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network)

add_subdirectory(common)
add_subdirectory(engineering_client)
add_subdirectory(production_client)
add_subdirectory(tests)"

create_file "README.md" "# Corporate Client"
create_file ".gitignore" "build/\n*.o\n*.user"

# Common library
create_dir "common/include/userserviceclient"
create_dir "common/src"

create_file "common/CMakeLists.txt" "cmake_minimum_required(VERSION 3.16)
project(UsersServiceClientLib)

find_package(Qt6 REQUIRED COMPONENTS Core Network)

add_library(UsersServiceClient STATIC)

target_include_directories(UsersServiceClient PUBLIC include)
target_sources(UsersServiceClient PRIVATE
    src/ApiClient.cpp
    src/AuthService.cpp
)

target_link_libraries(UsersServiceClient Qt6::Core Qt6::Network)"

create_empty_file "common/include/userserviceclient/ApiClient.h"
create_empty_file "common/src/ApiClient.cpp"

# Engineering client
create_dir "engineering_client"
create_file "engineering_client/CMakeLists.txt" "cmake_minimum_required(VERSION 3.16)
project(EngineeringClient)

add_executable(EngineeringClient main.cpp)
target_link_libraries(EngineeringClient UsersServiceClient Qt6::Core)"

create_file "engineering_client/main.cpp" "#include <QCoreApplication>\nint main() { return 0; }"

# Production client
create_dir "production_client/src/core"
create_dir "production_client/src/qml_bridge"
create_dir "production_client/qml/windows"

create_file "production_client/CMakeLists.txt" "cmake_minimum_required(VERSION 3.16)
project(ProductionClient)

find_package(Qt6 REQUIRED COMPONENTS Core Qml Quick)

add_executable(ProductionClient main.cpp)
target_link_libraries(ProductionClient Qt6::Core Qt6::Qml Qt6::Quick)"

create_file "production_client/main.cpp" "#include <QGuiApplication>\nint main() { return 0; }"

# Tests
create_dir "tests/unit"
create_file "tests/CMakeLists.txt" "cmake_minimum_required(VERSION 3.16)
project(Tests)

find_package(GTest REQUIRED)
add_executable(LibraryTests test_main.cpp)
target_link_libraries(LibraryTests GTest::GTest)"

create_file "tests/test_main.cpp" "#include <gtest/gtest.h>\nint main() { return 0; }"

echo "=== Done ==="
