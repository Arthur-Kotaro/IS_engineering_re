#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QDebug>
#include <memory>

#include "userserviceclient/ApiClient.h"
#include "userserviceclient/AuthService.h"
#include "src/qml_bridge/AuthBridge.h"
#include "src/qml_bridge/MainWindowBridge.h"

using namespace UsersService;

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName("Engineering :re");
    
    QQuickStyle::setStyle("Material");
    
    QQmlApplicationEngine engine;
    
    // Регистрируем Colors как синглтон
    qmlRegisterSingletonType(QUrl("qrc:/ProductionClient/qml/styles/Colors.qml"), "Styles", 1, 0, "Colors");
    
    auto apiClient = std::make_shared<ApiClient>();
    apiClient->setServerUrl("localhost", 8000);
    
    qDebug() << "Connecting to User Service at localhost:8000";
    
    auto authService = std::make_shared<AuthService>(apiClient);
    
    AuthBridge authBridge(authService);
    MainWindowBridge mainWindowBridge(authService);
    
    engine.rootContext()->setContextProperty("authBridge", &authBridge);
    engine.rootContext()->setContextProperty("mainWindowBridge", &mainWindowBridge);
    
    engine.load(QUrl("qrc:/ProductionClient/qml/main.qml"));
    
    if (engine.rootObjects().isEmpty()) {
        qDebug() << "Failed to load QML";
        return -1;
    }
    
    return app.exec();
}
