#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickItem>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QTimer>

#include "PreviewBridge.h"
#include "src/qml_bridge/WidgetBridge.h"
#include "src/core/ConfigManager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName("Engineering");
    app.setApplicationName("RendererPreview");

    QQmlApplicationEngine engine;

    qmlRegisterType<WidgetBridge>("ProductionClient", 1, 0, "WidgetBridge");
    
    // Регистрируем Colors как синглтон из файловой системы
    QString colorsPath = "/home/kotaro/code/IS_RE_engineering/CLIENT/production_client/qml/styles/Colors.qml";
    qmlRegisterSingletonType(QUrl::fromLocalFile(colorsPath), "Styles", 1, 0, "Colors");
    
    QString globalSettingsPath = "/home/kotaro/code/IS_RE_engineering/CLIENT/production_client/qml/styles/GlobalSettings.qml";
    qmlRegisterSingletonType(QUrl::fromLocalFile(globalSettingsPath), "Styles", 1, 0, "GlobalSettings");

    PreviewBridge previewBridge(&engine);
    engine.rootContext()->setContextProperty("previewBridge", &previewBridge);

    QStringList qmlPaths = {
        QCoreApplication::applicationDirPath() + "/PreviewWindow.qml",
        QCoreApplication::applicationDirPath() + "/../tools/renderer_preview/PreviewWindow.qml",
        QCoreApplication::applicationDirPath() + "/../../tools/renderer_preview/PreviewWindow.qml",
        QDir::currentPath() + "/PreviewWindow.qml",
        QDir::currentPath() + "/../tools/renderer_preview/PreviewWindow.qml",
        "/home/kotaro/code/IS_RE_engineering/CLIENT/production_client/tools/renderer_preview/PreviewWindow.qml"
    };

    QString qmlPath;
    for (const QString& path : qmlPaths) {
        if (QFile::exists(path)) {
            qmlPath = path;
            break;
        }
    }

    if (qmlPath.isEmpty()) {
        qCritical() << "PreviewWindow.qml not found in any path!";
        return -1;
    }

    qDebug() << "Loading QML from:" << qmlPath;

    QUrl url = QUrl::fromLocalFile(qmlPath);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl) {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}
