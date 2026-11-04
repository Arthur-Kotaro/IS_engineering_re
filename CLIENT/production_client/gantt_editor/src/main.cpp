#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QFile>
#include <QDebug>
#include <QQuickWindow>

#include "ProjectController.h"
#include "TaskModel.h"
#include "GroupModel.h"
#include "MilestoneModel.h"
#include "DependencyModel.h"
#include "ProjectData.h"
#include "ResourceManager.h"
#include "SettingsManager.h"
#include "ExportManager.h"
#include "HistoryEntry.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("GanttEditor");

    QCommandLineParser parser;
    parser.setApplicationDescription("Gantt Editor (МГ/ЛГ)");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption modeOption("mode", "Режим: master (МГ) или local (ЛГ).", "mode", "master");
    QCommandLineOption projectIdOption("project-id", "ID проекта.", "project-id", "0");
    QCommandLineOption mgIdOption("mg-id", "ID мастерграфика (UUID).", "mg-id", "");
    QCommandLineOption taskIdOption("task-id", "ID задачи (UUID) — для ЛГ.", "task-id", "");
    QCommandLineOption busUrlOption("bus-url", "URL BUS (для ЛГ).", "bus-url", "");
    QCommandLineOption tokenFileOption("token-file", "Файл с JWT-токеном.", "token-file", "");

    parser.addOption(modeOption);
    parser.addOption(projectIdOption);
    parser.addOption(mgIdOption);
    parser.addOption(taskIdOption);
    parser.addOption(busUrlOption);
    parser.addOption(tokenFileOption);
    parser.process(app);

    const QString mode = parser.value(modeOption);
    const int projectId = parser.value(projectIdOption).toInt();
    const QString mgId = parser.value(mgIdOption);
    const QString taskId = parser.value(taskIdOption);
    const QString busUrl = parser.value(busUrlOption);
    const QString tokenFile = parser.value(tokenFileOption);

    QString token;
    if (!tokenFile.isEmpty()) {
        QFile f(tokenFile);
        if (f.open(QIODevice::ReadOnly)) {
            token = QString::fromUtf8(f.readAll()).trimmed();
            f.close();
        } else {
            qWarning() << "Cannot read token file:" << tokenFile;
        }
    }

    qmlRegisterType<TaskModel>("GanttEditor", 1, 0, "TaskModel");
    qmlRegisterType<GroupModel>("GanttEditor", 1, 0, "GroupModel");
    qmlRegisterType<MilestoneModel>("GanttEditor", 1, 0, "MilestoneModel");
    qmlRegisterType<DependencyModel>("GanttEditor", 1, 0, "DependencyModel");
    qmlRegisterType<ProjectData>("GanttEditor", 1, 0, "ProjectData");
    qmlRegisterType<ResourceManager>("GanttEditor", 1, 0, "ResourceManager");
    qmlRegisterType<SettingsManager>("GanttEditor", 1, 0, "SettingsManager");
    qmlRegisterType<ExportManager>("GanttEditor", 1, 0, "ExportManager");
    qmlRegisterType<HistoryEntry>("GanttEditor", 1, 0, "HistoryEntry");

    ProjectController controller;

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("projectController", &controller);
    engine.rootContext()->setContextProperty("argMode", mode);
    engine.rootContext()->setContextProperty("argProjectId", projectId);
    engine.rootContext()->setContextProperty("argMgId", mgId);
    engine.rootContext()->setContextProperty("argTaskId", taskId);
    engine.rootContext()->setContextProperty("argBusUrl", busUrl);
    engine.rootContext()->setContextProperty("argToken", token);

    const QUrl url(QStringLiteral("qrc:/qml/GanttEditorWindow.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app,
                     [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    if (!engine.rootObjects().isEmpty()) {
        QQuickWindow *window = qobject_cast<QQuickWindow*>(engine.rootObjects().first());
        if (window)
            controller.get_exportManager()->setMainWindow(window);
    }

    return app.exec();
}
