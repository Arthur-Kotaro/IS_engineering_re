cmake_minimum_required(VERSION 3.16)
project(UsersServiceClient)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network)

set(SOURCES
    src/ApiClient.cpp
    src/AuthService.cpp
    src/TokenManager.cpp
    src/UserDataModels.cpp
    src/UserProfile.cpp
)

set(HEADERS
    include/userserviceclient/ApiClient.h
    include/userserviceclient/AuthService.h
    include/userserviceclient/TokenManager.h
    include/userserviceclient/UserDataModels.h
    include/userserviceclient/UserProfile.h
    include/userserviceclient/UsersServiceClient_global.h
)

add_library(UsersServiceClient STATIC ${SOURCES} ${HEADERS})
target_include_directories(UsersServiceClient PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(UsersServiceClient Qt6::Core Qt6::Network)
