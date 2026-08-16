cmake_minimum_required(VERSION 3.16)
project(RendererPreview)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Qml Quick QuickControls2 Widgets)

add_executable(renderer_preview
    main.cpp
    PreviewBridge.cpp
)

target_link_libraries(renderer_preview
    PRIVATE
        RendererLib
        Qt6::Core
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Widgets
)

target_include_directories(renderer_preview PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)

add_custom_command(TARGET renderer_preview POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/samples
        ${CMAKE_CURRENT_BINARY_DIR}/samples
    COMMENT "Copying sample JSON files"
)

install(TARGETS renderer_preview RUNTIME DESTINATION bin)
