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
    
    // 1. Загружаем конфиг ДО создания engine
    ConfigManager* config = ConfigManager::instance();
    
    // 2. Применяем шрифт к приложению
    QFont defaultFont = app.font();
    defaultFont.setPixelSize(config->fontSize());
    app.setFont(defaultFont);
    
    // 3. Создаём engine после применения настроек
    QQmlApplicationEngine engine;
    
    qmlRegisterType<WidgetBridge>("ProductionClient", 1, 0, "WidgetBridge");
    
    AppCore core(&engine);
    core.init();
    
    auto authService = AppCore::authService();
    
    AuthBridge authBridge(authService);
    MainWindowBridge mainWindowBridge(authService);
    NotificationBridge notificationBridge;
    WidgetBridge widgetBridge;
    
    widgetBridge.setDataManager(core.dataManager());
    widgetBridge.setRenderer(core.renderer());
    
    // 4. Регистрируем Colors как синглтон
    qmlRegisterSingletonType(QUrl("qrc:/ProductionClient/qml/styles/Colors.qml"), "Styles", 1, 0, "Colors");
    
    // 5. Передаём config в QML контекст
    engine.rootContext()->setContextProperty("appCore", &core);
    engine.rootContext()->setContextProperty("authBridge", &authBridge);
    engine.rootContext()->setContextProperty("mainWindowBridge", &mainWindowBridge);
    engine.rootContext()->setContextProperty("notificationBridge", &notificationBridge);
    engine.rootContext()->setContextProperty("widgetBridge", &widgetBridge);
    engine.rootContext()->setContextProperty("renderer", core.renderer());
    engine.rootContext()->setContextProperty("configManager", config);
    
    // 6. Загружаем QML
    QString qmlPath = QCoreApplication::applicationDirPath() + "/qml/main.qml";
    if (!QFile::exists(qmlPath)) {
        qmlPath = QDir::currentPath() + "/qml/main.qml";
    }
    if (!QFile::exists(qmlPath)) {
        qmlPath = QString("%1/../production_client/qml/main.qml")
            .arg(QCoreApplication::applicationDirPath());
    }
    
    qDebug() << "Loading QML from:" << qmlPath;
    
    QUrl url = QUrl::fromLocalFile(qmlPath);
    engine.load(url);
    
    return app.exec();
}
