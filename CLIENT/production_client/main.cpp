#include "src/core/AppCore.h"
#include "src/core/ConfigManager.h"
#include "src/qml_bridge/AuthBridge.h"
#include "src/qml_bridge/MainWindowBridge.h"
#include "src/qml_bridge/NotificationBridge.h"
#include "src/qml_bridge/WidgetBridge.h"
#include "src/renderer/JsonUiRenderer.h"

#include "gantt_editor/include/ProjectController.h"
#include "gantt_editor/include/ProjectData.h"
#include "gantt_editor/include/TaskModel.h"
#include "gantt_editor/include/GroupModel.h"
#include "gantt_editor/include/MilestoneModel.h"
#include "gantt_editor/include/DependencyModel.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QFont>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setOrganizationName("Engineering");
    app.setApplicationName("EngineeringRE");

    ConfigManager *config = ConfigManager::instance();

    QFont defaultFont = app.font();
    defaultFont.setPixelSize(config->fontSize());
    app.setFont(defaultFont);

    QQmlApplicationEngine engine;

    qmlRegisterType<WidgetBridge>("ProductionClient", 1, 0, "WidgetBridge");

    // Регистрация типов Gantt Editor (GDERE)
    qmlRegisterType<GanttEditor::ProjectController>("GanttEditor", 1, 0, "ProjectController");
    qmlRegisterType<GanttEditor::ProjectData>("GanttEditor", 1, 0, "ProjectData");
    qmlRegisterType<GanttEditor::TaskModel>("GanttEditor", 1, 0, "TaskModel");
    qmlRegisterType<GanttEditor::GroupModel>("GanttEditor", 1, 0, "GroupModel");
    qmlRegisterType<GanttEditor::MilestoneModel>("GanttEditor", 1, 0, "MilestoneModel");
    qmlRegisterType<GanttEditor::DependencyModel>("GanttEditor", 1, 0, "DependencyModel");
    
    // Регистрация QML-файлов Gantt Editor
    qmlRegisterType(QUrl("qrc:/gantt_editor/qml/GanttEditorWindow.qml"),
                    "GanttEditor", 1, 0, "GanttEditorWindow");

    AppCore core(&engine);
    core.init();

    auto authService = AppCore::authService();

    AuthBridge authBridge(authService);
    MainWindowBridge mainWindowBridge(authService);
    NotificationBridge notificationBridge;
    WidgetBridge widgetBridge;

    // Создаём контроллер для Gantt Editor
    GanttEditor::ProjectController ganttController;

    widgetBridge.setDataManager(core.dataManager());
    widgetBridge.setRenderer(core.renderer());

    qmlRegisterType(QUrl("qrc:/ProductionClient/qml/widgets/SearchableListSingle.qml"),
                    "BaseWidgets", 1, 0, "SearchableListSingle");
    qmlRegisterType(QUrl("qrc:/ProductionClient/qml/widgets/SearchableListMulti.qml"),
                    "BaseWidgets", 1, 0, "SearchableListMulti");
    qmlRegisterType(QUrl("qrc:/ProductionClient/qml/widgets/SearchableSelect.qml"), "BaseWidgets",
                    1, 0, "SearchableSelect");

    qmlRegisterType(QUrl("qrc:/ProductionClient/qml/dialogs/ValidationErrorDialog.qml"), "Dialogs",
                    1, 0, "ValidationErrorDialog");

    engine.rootContext()->setContextProperty("appCore", &core);
    engine.rootContext()->setContextProperty("authBridge", &authBridge);
    engine.rootContext()->setContextProperty("mainWindowBridge", &mainWindowBridge);
    engine.rootContext()->setContextProperty("notificationBridge", &notificationBridge);
    engine.rootContext()->setContextProperty("widgetBridge", &widgetBridge);
    engine.rootContext()->setContextProperty("renderer", core.renderer());
    engine.rootContext()->setContextProperty("configManager", config);
    engine.rootContext()->setContextProperty("projectController", &ganttController);

    QString qmlPath = QCoreApplication::applicationDirPath() + "/qml/main.qml";
    if (!QFile::exists(qmlPath))
    {
        qmlPath = QDir::currentPath() + "/qml/main.qml";
    }
    if (!QFile::exists(qmlPath))
    {
        qmlPath = QString("%1/../production_client/qml/main.qml")
                      .arg(QCoreApplication::applicationDirPath());
    }

    qDebug() << "Loading QML from:" << qmlPath;
    engine.load(QUrl::fromLocalFile(qmlPath));

    return app.exec();
}
