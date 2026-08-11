cmake_minimum_required(VERSION 3.16)
project(ClientTests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network Test)
find_package(GTest REQUIRED)

# НЕ добавляем common здесь - он уже добавлен в корневом CMakeLists.txt

# Включаем директории заголовков
include_directories(
    ${CMAKE_SOURCE_DIR}/common/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Тестовые утилиты
add_library(TestUtils STATIC
    utils/MockApiClient.cpp
    utils/TestHelpers.cpp
)

target_link_libraries(TestUtils
    PUBLIC
        UsersServiceClient
        GTest::gtest
        GTest::gtest_main
        Qt6::Core
        Qt6::Test
)

# Тесты библиотеки
add_executable(LibraryTests
    test_main.cpp
    unit/TestTokenManager.cpp
    unit/TestUserDataModels.cpp
    unit/TestConfigManager.cpp
    unit/TestCacheManager.cpp
    integration/TestAuthService.cpp
    integration/TestApiClient.cpp
    integration/TestMastergraphService.cpp
)

target_link_libraries(LibraryTests
    PRIVATE
        UsersServiceClient
        TestUtils
        GTest::gtest
        GTest::gtest_main
        Qt6::Core
        Qt6::Test
        Qt6::Network
)

# Тесты инженерного клиента
add_executable(EngineeringClientTests
    engineering/TestEngineeringConsole.cpp
)

target_link_libraries(EngineeringClientTests
    PRIVATE
        UsersServiceClient
        TestUtils
        GTest::gtest
        GTest::gtest_main
        Qt6::Core
        Qt6::Test
)

# Регистрируем тесты
add_test(NAME LibraryTests COMMAND LibraryTests)
add_test(NAME EngineeringClientTests COMMAND EngineeringClientTests)

# Настройки для покрытия кода
if(ENABLE_COVERAGE)
    target_compile_options(LibraryTests PRIVATE --coverage -fprofile-arcs -ftest-coverage)
    target_link_options(LibraryTests PRIVATE --coverage)
endif()
