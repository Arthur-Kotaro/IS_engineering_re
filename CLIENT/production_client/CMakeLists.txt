cmake_minimum_required(VERSION 3.16)
project(ProductionClient)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network Qml Quick QuickControls2)

# Убираем add_subdirectory(common) - он уже добавлен в корневом CMakeLists.txt

# QML ресурсы
set(QML_FILES
    qml/main.qml
    qml/windows/AuthWindow.qml
    qml/windows/MainWindow.qml
    qml/windows/SplashScreen.qml
    #   qml/components/TopBar.qml     #FUNCTIONALITY migrated to MainWindow
    qml/components/WidgetLoader.qml
    qml/components/ToastNotification.qml
    qml/components/LoadingSpinner.qml
    qml/components/PasswordExpiryWarning.qml
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

# Общие виджеты
file(GLOB COMMON_WIDGETS "qml/widgets/common/*.qml")
list(APPEND QML_FILES ${COMMON_WIDGETS})

# Виджеты по ролям
file(GLOB_RECURSE ROLE_WIDGETS "qml/widgets/roles/*.qml")
list(APPEND QML_FILES ${ROLE_WIDGETS})

# Регистрируем QML модуль
qt_add_qml_module(ProductionClient
    URI ProductionClient
    VERSION 1.0
    QML_FILES ${QML_FILES}
)

# Создаём исполняемый файл
add_executable(ProductionClientApp
    main.cpp
    src/qml_bridge/AuthBridge.cpp
    src/qml_bridge/MainWindowBridge.cpp
    src/qml_bridge/NotificationBridge.cpp
    src/core/AppCore.cpp
    src/core/ConfigManager.cpp
    src/core/CacheManager.cpp
)

target_link_libraries(ProductionClientApp
    PRIVATE
        ProductionClient
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
    ${CMAKE_SOURCE_DIR}/common/include
)

install(TARGETS ProductionClientApp
    RUNTIME DESTINATION bin
)
