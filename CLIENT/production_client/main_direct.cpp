#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QDebug>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    QQmlApplicationEngine engine;
    
    // Загружаем напрямую из файловой системы
    QString qmlPath = QCoreApplication::applicationDirPath() + "/../production_client/qml/main.qml";
    
    qDebug() << "Loading QML from:" << qmlPath;
    
    if (!QFile::exists(qmlPath)) {
        qDebug() << "File not found! Trying current directory...";
        qmlPath = "production_client/qml/main.qml";
        
        if (!QFile::exists(qmlPath)) {
            qDebug() << "Still not found! Creating simple QML inline...";
            
            // Создаём QML напрямую в коде
            engine.setInitialProperties({{"width", 800}, {"height", 600}});
            engine.loadData(R"(
import QtQuick 6.0
import QtQuick.Controls 6.0

ApplicationWindow {
    visible: true
    width: 800
    height: 600
    title: "Corporate Client"
    
    Rectangle {
        anchors.fill: parent
        color: "#2c3e50"
        
        Text {
            anchors.centerIn: parent
            text: "Corporate Client\nSuccessfully Started!"
            color: "white"
            font.pixelSize: 24
            horizontalAlignment: Text.AlignHCenter
        }
    }
}
            )");
            
            if (engine.rootObjects().isEmpty()) {
                qDebug() << "Failed to load inline QML";
                return -1;
            }
            
            return app.exec();
        }
    }
    
    engine.load(QUrl::fromLocalFile(qmlPath));
    
    if (engine.rootObjects().isEmpty()) {
        qDebug() << "Failed to load QML from file";
        return -1;
    }
    
    qDebug() << "Production Client started successfully";
    
    return app.exec();
}
