cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Network
    Qml
    Quick
    QuickControls2
    WebSockets
    PrintSupport
    Widgets
)

qt6_standard_project_setup()

# ---------- GanttEditorLib (общая библиотека) ----------
set(GANTT_SOURCES
    src/TaskModel.cpp
    src/GroupModel.cpp
    src/MilestoneModel.cpp
    src/DependencyModel.cpp
    src/HistoryEntry.cpp
    src/ProjectData.cpp
    src/ProjectSerializer.cpp
    src/ProjectValidator.cpp
    src/ProjectController.cpp
    src/TaskController.cpp
    src/DependencyController.cpp
    src/LocalGraphController.cpp
    src/SettingsManager.cpp
    src/ResourceManager.cpp
    src/ExportManager.cpp
    src/GanttApiClient.cpp
)

set(GANTT_HEADERS
    include/TaskModel.h
    include/GroupModel.h
    include/MilestoneModel.h
    include/DependencyModel.h
    include/HistoryEntry.h
    include/GlobalDefines.h
    include/ProjectData.h
    include/ProjectSerializer.h
    include/ProjectValidator.h
    include/ProjectController.h
    include/TaskController.h
    include/DependencyController.h
    include/LocalGraphController.h
    include/SettingsManager.h
    include/ResourceManager.h
    include/ExportManager.h
    include/GanttApiClient.h
)

add_library(GanttEditorLib STATIC ${GANTT_SOURCES} ${GANTT_HEADERS})

target_include_directories(GanttEditorLib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(GanttEditorLib PUBLIC
    Qt6::Core
    Qt6::Network
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::WebSockets
    Qt6::Widgets
)

# ---------- qml.qrc ----------
set(GANTT_QML_QRC ${CMAKE_CURRENT_SOURCE_DIR}/qml.qrc)

# ---------- gantt_editor ----------
add_executable(gantt_editor
    src/main.cpp
    ${GANTT_QML_QRC}
)

target_link_libraries(gantt_editor PRIVATE
    GanttEditorLib
    Qt6::Widgets
)

# ---------- export_renderer ----------
add_executable(export_renderer
    export_renderer/main.cpp
    ${GANTT_QML_QRC}
)

target_link_libraries(export_renderer PRIVATE
    GanttEditorLib
    Qt6::PrintSupport
    Qt6::Widgets
)

# ---------- Копирование ресурсов в build/ ----------
set(RESOURCE_FILES
    resources/default_typologies.json
    resources/default_tasks.json
    resources/default_holidays.json
)

foreach(RES_FILE ${RESOURCE_FILES})
    get_filename_component(RES_NAME ${RES_FILE} NAME)
    add_custom_command(
        TARGET gantt_editor POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/${RES_FILE}
            $<TARGET_FILE_DIR:gantt_editor>/${RES_NAME}
        COMMENT "Copying ${RES_NAME} to gantt_editor build dir"
    )
    add_custom_command(
        TARGET export_renderer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_CURRENT_SOURCE_DIR}/${RES_FILE}
            $<TARGET_FILE_DIR:export_renderer>/${RES_NAME}
        COMMENT "Copying ${RES_NAME} to export_renderer build dir"
    )
endforeach()
