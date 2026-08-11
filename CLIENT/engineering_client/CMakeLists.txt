cmake_minimum_required(VERSION 3.16)
project(EngineeringUserServiceClient)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network)

# Используем общую библиотеку (уже добавлена в корневом CMakeLists.txt)
add_executable(EngineeringClient
    main.cpp
)

target_link_libraries(EngineeringClient
    PRIVATE
        UsersServiceClient
        Qt6::Core
        Qt6::Network
)

target_include_directories(EngineeringClient PRIVATE
    ${CMAKE_SOURCE_DIR}/common/include
)

if(UNIX AND NOT APPLE)
    find_package(OpenSSL QUIET)
    if(OPENSSL_FOUND)
        target_link_libraries(EngineeringClient PRIVATE ${OPENSSL_LIBRARIES})
    endif()
endif()

install(TARGETS EngineeringClient
    RUNTIME DESTINATION bin
)
