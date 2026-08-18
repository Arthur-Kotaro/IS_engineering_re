#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QIcon>
#include <QDebug>
#include <QDir>
#include <QFileInfo>

#include "src/core/AppCore.h"
#include "src/qml_bridge/AuthBridge.h"
#include "src/qml_bridge/MainWindowBridge.h"
#include "src/qml_bridge/NotificationBridge.h"
#include "src/qml_bridge/WidgetBridge.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setOrganizationName("Engineering");
    app.setApplicationName("EngineeringRE");
    
    QQmlApplicationEngine engine;
    
    // Регистрируем WidgetBridge для QML
    qmlRegisterType<WidgetBridge>("ProductionClient", 1, 0, "WidgetBridge");
    
    // Инициализация AppCore с engine
    AppCore core(&engine);
    core.init();
    
    // Получаем сервисы
    auto authService = AppCore::authService();
    
    // Создание мостов
    AuthBridge authBridge(authService);
    MainWindowBridge mainWindowBridge(authService);
    NotificationBridge notificationBridge;
    WidgetBridge widgetBridge;
    
    // Связываем WidgetBridge с AppCore
    widgetBridge.setDataManager(core.dataManager());
    widgetBridge.setRenderer(core.renderer());
    
    // Регистрируем Colors как синглтон
    qmlRegisterSingletonType(QUrl("qrc:/ProductionClient/qml/styles/Colors.qml"), "Styles", 1, 0, "Colors");
    
    // Экспортируем в QML
    engine.rootContext()->setContextProperty("appCore", &core);
    engine.rootContext()->setContextProperty("authBridge", &authBridge);
    engine.rootContext()->setContextProperty("mainWindowBridge", &mainWindowBridge);
    engine.rootContext()->setContextProperty("notificationBridge", &notificationBridge);
    engine.rootContext()->setContextProperty("widgetBridge", &widgetBridge);
    
    // Загружаем QML
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
