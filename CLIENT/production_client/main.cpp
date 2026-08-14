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

// Регистрируем Colors как синглтон
#include <QQmlEngine>
#include <QQmlComponent>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setOrganizationName("Engineering");
    app.setApplicationName("EngineeringRE");
    
    // Инициализация AppCore
    AppCore core;
    core.init();
    
    auto authService = getAuthService();
    
    // Создание мостов
    AuthBridge authBridge(authService);
    MainWindowBridge mainWindowBridge(authService);
    NotificationBridge notificationBridge;
    
    QQmlApplicationEngine engine;
    
    // Регистрируем Colors как синглтон (из старой версии)
    qmlRegisterSingletonType(QUrl("qrc:/ProductionClient/qml/styles/Colors.qml"), "Styles", 1, 0, "Colors");
    
    engine.rootContext()->setContextProperty("authBridge", &authBridge);
    engine.rootContext()->setContextProperty("mainWindowBridge", &mainWindowBridge);
    engine.rootContext()->setContextProperty("notificationBridge", &notificationBridge);
    
    // Загружаем QML из файловой системы (из новой версии)
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
    
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    
    engine.load(url);
    
    return app.exec();
}
