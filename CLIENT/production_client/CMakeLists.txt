cmake_minimum_required(VERSION 3.16)
project(ProductionClient)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network Qml Quick QuickControls2)

set(COMMON_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../common)
set(COMMON_INCLUDE ${COMMON_DIR}/include)

set(RENDERER_SOURCES
    src/renderer/JsonUiRenderer.cpp
    src/renderer/QmlObjectFactory.cpp
    src/renderer/qml/GeneratorHelpers.cpp
    src/renderer/qml/ChartGenerators.cpp
    src/renderer/qml/TableGenerators.cpp
    src/renderer/qml/LayoutGenerators.cpp
    src/renderer/qml/WidgetGenerators.cpp
    src/core/DataManager.cpp
)

set(RENDERER_HEADERS
    src/renderer/JsonUiRenderer.h
    src/renderer/QmlObjectFactory.h
    src/renderer/qml/GeneratorHelpers.h
    src/core/DataManager.h
)

add_library(RendererLib STATIC
    ${RENDERER_SOURCES}
    ${RENDERER_HEADERS}
)

target_include_directories(RendererLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer
    ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/qml
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
)

target_link_libraries(RendererLib
    PUBLIC
        Qt6::Core
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Network
)

set(QML_FILES
    qml/main.qml
    qml/windows/AuthWindow.qml
    qml/windows/MainWindow.qml
    qml/windows/SplashScreen.qml
    qml/components/WidgetLoader.qml
    qml/components/ToastNotification.qml
    qml/components/TabContent.qml
    qml/dialogs/ChangePasswordDialog.qml
    qml/dialogs/ConfirmDialog.qml
    qml/dialogs/ErrorDialog.qml
    qml/styles/AppStyle.qml
    qml/styles/Colors.qml
    qml/styles/Typography.qml
    qml/styles/Button.qml
    qml/utils/DateUtils.qml
    qml/utils/StringUtils.qml
    qml/utils/Validators.qml
)

file(GLOB COMMON_WIDGETS "qml/widgets/common/*.qml")
list(APPEND QML_FILES ${COMMON_WIDGETS})

file(GLOB_RECURSE ROLE_WIDGETS "qml/widgets/roles/*.qml")
list(APPEND QML_FILES ${ROLE_WIDGETS})

qt_add_qml_module(ProductionClient
    URI ProductionClient
    VERSION 1.0
    QML_FILES ${QML_FILES}
)

add_executable(ProductionClientApp
    main.cpp
    src/qml_bridge/AuthBridge.cpp
    src/qml_bridge/MainWindowBridge.cpp
    src/qml_bridge/NotificationBridge.cpp
    src/qml_bridge/WidgetBridge.cpp
    src/core/AppCore.cpp
    src/core/ConfigManager.cpp
    src/core/CacheManager.cpp
)

target_link_libraries(ProductionClientApp
    PRIVATE
        ProductionClient
        RendererLib
        UsersServiceClient
        Qt6::Core
        Qt6::Network
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
)

target_include_directories(ProductionClientApp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/qml_bridge
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/renderer
    ${COMMON_INCLUDE}
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_subdirectory(tools/renderer_preview)
endif()

install(TARGETS ProductionClientApp RUNTIME DESTINATION bin)
