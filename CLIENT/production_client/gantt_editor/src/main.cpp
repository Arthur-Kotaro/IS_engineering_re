#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QFile>
#include <QDebug>
#include <QQuickWindow>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QByteArray>
#include <QMessageBox>
#include <QProcessEnvironment>

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

static int decode_user_id(const QString& token)
{
    if (token.isEmpty()) return 0;
    QStringList parts = token.split('.');
    if (parts.size() != 3) return 0;
    QByteArray b = parts[1].toUtf8();
    b.replace('-', '+').replace('_', '/');
    while (b.size() % 4) b.append('=');
    QByteArray payload = QByteArray::fromBase64(b);
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(payload, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) return 0;
    return doc.object().value("user_id").toInt(0);
}

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
    QCommandLineOption readOnlyOption("read-only", "Открыть в режиме просмотра.");

    parser.addOption(modeOption);
    parser.addOption(projectIdOption);
    parser.addOption(mgIdOption);
    parser.addOption(taskIdOption);
    parser.addOption(busUrlOption);
    parser.addOption(tokenFileOption);
    parser.addOption(readOnlyOption);
    parser.process(app);

    const QString mode = parser.value(modeOption);
    const int projectId = parser.value(projectIdOption).toInt();
    const QString mgId = parser.value(mgIdOption);
    const QString taskId = parser.value(taskIdOption);
    const QString busUrl = parser.value(busUrlOption);
    const QString tokenFile = parser.value(tokenFileOption);
    const bool isReadOnly = parser.isSet(readOnlyOption);

    // 1. Приоритет: переменная окружения GANTT_TOKEN (безопаснее файла)
    QString token = qEnvironmentVariable("GANTT_TOKEN");

    // 2. Fallback: --token-file (с удалением после чтения)
    if (token.isEmpty() && !tokenFile.isEmpty()) {
        QFile f(tokenFile);
        if (f.open(QIODevice::ReadOnly)) {
            token = QString::fromUtf8(f.readAll()).trimmed();
            f.close();
        }
        // ВАЖНО: удаляем временный файл после чтения
        QFile::remove(tokenFile);
    }

    const int currentUserId = decode_user_id(token);

    qInfo().noquote() << "========== GANTT EDITOR START =========="
                      << "\n  mode:" << mode
                      << "\n  projectId:" << projectId
                      << "\n  mgId:" << mgId
                      << "\n  taskId:" << taskId
                      << "\n  busUrl:" << busUrl
                      << "\n  tokenFile:" << tokenFile
                      << "\n  token.length:" << token.length()
                      << "\n  currentUserId:" << currentUserId
                      << "\n  isReadOnly:" << isReadOnly;

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
    engine.rootContext()->setContextProperty("argReadOnly", isReadOnly);

    QUrl url;
    if (mode == "local") {
        url = QUrl(QStringLiteral("qrc:/qml/LocalGraphEditorWindow.qml"));

        QNetworkAccessManager mgr;
        const QString byTaskUrl = busUrl + "/internal/local-graphs/by-task/" + taskId;
        qInfo().noquote() << "[LG] GET by-task:" << byTaskUrl << " X-User-ID:" << currentUserId;
        QNetworkRequest req{QUrl(byTaskUrl)};
        req.setRawHeader("X-User-ID", QByteArray::number(currentUserId));
        if (!token.isEmpty()) {
            req.setRawHeader("Authorization", "Bearer " + token.toUtf8());
        }
        QNetworkReply* reply = mgr.get(req);
        QEventLoop loop;
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QByteArray body = reply->readAll();
        qInfo().noquote() << "[LG] by-task response:"
                          << "\n  httpStatus:" << httpStatus
                          << "\n  error:" << reply->error()
                          << "\n  errorString:" << reply->errorString()
                          << "\n  body:" << QString::fromUtf8(body);

        if (reply->error() == QNetworkReply::ContentNotFoundError) {
            QMessageBox::warning(nullptr, "Локальный график",
                                 "Для этой задачи локальный график не создан.");
            return 0;
        }
        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox::warning(nullptr, "Локальный график",
                                 "Ошибка загрузки ЛГ: " + reply->errorString());
            return 0;
        }

        QJsonDocument doc = QJsonDocument::fromJson(body);
        if (!doc.isObject()) {
            QMessageBox::warning(nullptr, "Локальный график", "Некорректный ответ BUS");
            return 0;
        }
        QVariantMap lgData = doc.object().toVariantMap();
        QString lgId = lgData.value("lg_id").toString();
        int lgProjectId = lgData.value("project_id").toInt();
        int lgResponsible = lgData.value("responsible_id").toInt();
        bool canEdit = lgData.value("can_edit").toBool();

        qInfo().noquote() << "[LG] lgData:"
                          << "\n  lg_id:" << lgId
                          << "\n  project_id:" << lgProjectId
                          << "\n  responsible_id:" << lgResponsible
                          << "\n  can_edit:" << canEdit
                          << "\n  target_start:" << lgData.value("target_start")
                          << "\n  target_end:" << lgData.value("target_end")
                          << "\n  predecessor_end:" << lgData.value("predecessor_end")
                          << "\n  is_pure:" << lgData.value("is_pure")
                          << "\n  local_graph_state:" << lgData.value("local_graph_state");

        controller.get_localGraphController()->setLocalGraphContext(
            lgId, busUrl, lgProjectId, lgResponsible, taskId);

        const QString tasksUrl = busUrl + "/internal/local-graphs/" + lgId + "/tasks";
        qInfo().noquote() << "[LG] GET tasks:" << tasksUrl;
        QNetworkRequest tReq{QUrl(tasksUrl)};
        tReq.setRawHeader("X-User-ID", QByteArray::number(currentUserId));
        if (!token.isEmpty()) {
            tReq.setRawHeader("Authorization", "Bearer " + token.toUtf8());
        }
        QNetworkReply* tReply = mgr.get(tReq);
        QEventLoop tLoop;
        QObject::connect(tReply, &QNetworkReply::finished, &tLoop, &QEventLoop::quit);
        tLoop.exec();

        int tHttpStatus = tReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QByteArray tBody = tReply->readAll();
        qInfo().noquote() << "[LG] tasks response:"
                          << "\n  httpStatus:" << tHttpStatus
                          << "\n  error:" << tReply->error()
                          << "\n  errorString:" << tReply->errorString()
                          << "\n  body:" << QString::fromUtf8(tBody);

        QVariantList tasks;
        if (tReply->error() == QNetworkReply::NoError) {
            QJsonDocument tDoc = QJsonDocument::fromJson(tBody);
            if (tDoc.isObject()) {
                tasks = tDoc.object().value("tasks").toArray().toVariantList();
            }
        }
        tReply->deleteLater();
        reply->deleteLater();

        qInfo() << "[LG] tasks.size:" << tasks.size()
                << " — calling initLocalGraphData";

        controller.initLocalGraphData(lgData, tasks, busUrl);
        qInfo() << "[LG] initLocalGraphData done. Loading QML:" << url.toString();
    } else {
        url = QUrl(QStringLiteral("qrc:/qml/GanttEditorWindow.qml"));

        if (mgId.isEmpty() && projectId > 0) {
            controller.openForProject(projectId, isReadOnly, token);
        } else if (!mgId.isEmpty()) {
            controller.loadFromServer(mgId, token);
        }
    }

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated, &app,
                     [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl) {
            qWarning() << "[GanttEditor] objectCreated ERROR, url:" << objUrl;
            QCoreApplication::exit(-1);
        }
    }, Qt::QueuedConnection);
    engine.load(url);

    if (engine.rootObjects().isEmpty()) {
        qWarning() << "[GanttEditor] engine.rootObjects().isEmpty() — QML load FAILED";
    } else {
        qInfo() << "[GanttEditor] engine.load OK, root objects:" << engine.rootObjects().size();
    }

    if (!engine.rootObjects().isEmpty()) {
        QQuickWindow *window = qobject_cast<QQuickWindow*>(engine.rootObjects().first());
        if (window)
            controller.get_exportManager()->setMainWindow(window);
    }

    return app.exec();
}
