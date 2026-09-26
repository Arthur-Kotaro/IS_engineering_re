#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QFont>
#include <QDebug>
#include <QDir>
#include <QFileInfo>

#include "src/core/AppCore.h"
#include "src/core/ConfigManager.h"
#include "src/qml_bridge/AuthBridge.h"
#include "src/qml_bridge/MainWindowBridge.h"
#include "src/qml_bridge/NotificationBridge.h"
#include "src/qml_bridge/WidgetBridge.h"
#include "src/renderer/JsonUiRenderer.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setOrganizationName("Engineering");
    app.setApplicationName("EngineeringRE");
    
    ConfigManager* config = ConfigManager::instance();
    
    QFont defaultFont = app.font();
    defaultFont.setPixelSize(config->fontSize());
    app.setFont(defaultFont);
    
    QQmlApplicationEngine engine;
    
    qmlRegisterType<WidgetBridge>("ProductionClient", 1, 0, "WidgetBridge");
    qmlRegisterSingletonType(QUrl("qrc:/ProductionClient/qml/styles/Colors.qml"), "Styles", 1, 0, "Colors");
    qmlRegisterSingletonType(QUrl("qrc:/ProductionClient/qml/styles/GlobalSettings.qml"), "Styles", 1, 0, "GlobalSettings");
    
    AppCore core(&engine);
    core.init();
    
    auto authService = AppCore::authService();
    
    AuthBridge authBridge(authService);
    MainWindowBridge mainWindowBridge(authService);
    NotificationBridge notificationBridge;
    WidgetBridge widgetBridge;
    
    widgetBridge.setDataManager(core.dataManager());
    widgetBridge.setRenderer(core.renderer());
    
    engine.rootContext()->setContextProperty("appCore", &core);
    engine.rootContext()->setContextProperty("authBridge", &authBridge);
    engine.rootContext()->setContextProperty("mainWindowBridge", &mainWindowBridge);
    engine.rootContext()->setContextProperty("notificationBridge", &notificationBridge);
    engine.rootContext()->setContextProperty("widgetBridge", &widgetBridge);
    engine.rootContext()->setContextProperty("renderer", core.renderer());
    engine.rootContext()->setContextProperty("configManager", config);
    
    engine.load(QUrl("qrc:/ProductionClient/qml/main.qml"));
    
    return app.exec();
}
