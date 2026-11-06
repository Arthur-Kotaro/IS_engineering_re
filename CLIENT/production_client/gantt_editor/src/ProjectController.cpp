#include "ProjectController.h"
#include <QDebug>
#include <QGuiApplication>
#include <QCursor>
#include <QDate>
#include <QJsonArray>
#include <QJsonObject>
#include <QProcess>
#include <QFile>
#include <QCoreApplication>

ProjectController::ProjectController(QObject *parent)
    : QObject(parent)
    , m_projectData(nullptr)
    , m_inEditMode(false)
{
    m_projectData = new ProjectData(this);
    m_resourceManager = new ResourceManager(this);
    m_settingsManager = new SettingsManager(this);
    m_exportManager = new ExportManager(this);
    m_apiClient = new GanttApiClient(this);
    m_webSocket = new WebSocketClient(this);

    m_taskController = new TaskController(m_projectData, m_settingsManager, this);
    m_localGraphController = new LocalGraphController(m_projectData, m_resourceManager, m_settingsManager, m_apiClient, this);
    m_dependencyController = new DependencyController(m_projectData, m_settingsManager, this);

    m_resourceManager->setResourcesPath(m_settingsManager->resourcesPath());

    connect(m_taskController, &TaskController::errorOccurred, this, &ProjectController::errorOccurred);
    connect(m_localGraphController, &LocalGraphController::errorOccurred, this, &ProjectController::errorOccurred);
    connect(m_dependencyController, &DependencyController::errorOccurred, this, &ProjectController::errorOccurred);
    connect(m_taskController, &TaskController::confirmRemoveTaskWithLocalGraph, this, &ProjectController::confirmRemoveTaskWithLocalGraph);

    connect(m_apiClient, &GanttApiClient::mastergraphLoaded, this, &ProjectController::onMastergraphLoaded);
    connect(m_apiClient, &GanttApiClient::accessLoaded, this, &ProjectController::onAccessLoaded);
    connect(m_apiClient, &GanttApiClient::holidaysLoaded, this, &ProjectController::onHolidaysLoaded);
    connect(m_apiClient, &GanttApiClient::localGraphLoaded, this, &ProjectController::onLocalGraphLoaded);
    connect(m_apiClient, &GanttApiClient::errorOccurred, this, &ProjectController::onApiError);

    connect(m_webSocket, &WebSocketClient::snapshotReceived, this, &ProjectController::onWsSnapshot);
    connect(m_webSocket, &WebSocketClient::diffReceived, this, &ProjectController::onWsDiff);

    m_apiClient->setBaseUrl("http://localhost:8080");
}

ProjectData* ProjectController::get_projectData() const { return m_projectData; }
ResourceManager* ProjectController::get_resourceManager() const { return m_resourceManager; }
SettingsManager* ProjectController::get_settingsManager() const { return m_settingsManager; }
ExportManager* ProjectController::get_exportManager() const { return m_exportManager; }
TaskController* ProjectController::get_taskController() const { return m_taskController; }
LocalGraphController* ProjectController::get_localGraphController() const { return m_localGraphController; }
DependencyController* ProjectController::get_dependencyController() const { return m_dependencyController; }
WebSocketClient* ProjectController::get_webSocket() const { return m_webSocket; }

bool ProjectController::get_inEditMode() const { return m_inEditMode; }

void ProjectController::set_InEditMode(bool editMode)
{
    if (m_inEditMode != editMode) {
        m_inEditMode = editMode;
        emit inEditModeChanged();
    }
}

void ProjectController::loadFromServer(const QString& mgId, const QString& token)
{
    qDebug() << "=== loadFromServer called: mgId=" << mgId;
    if (mgId.isEmpty()) {
        emit errorOccurred("MG ID is empty");
        return;
    }
    m_token = token;
    m_apiClient->setToken(token);
    m_apiClient->loadMastergraph(mgId);
    m_apiClient->loadHolidays();

    m_webSocket->connectToServer(
        QString("ws://localhost:8080/ws/mastergraphs/%1").arg(mgId),
        token
    );
}

void ProjectController::openLocalGraphProcess(const QString& taskId, const QString& busUrl)
{
    qDebug() << "=== openLocalGraphProcess:" << taskId << busUrl;

    QString program = QCoreApplication::applicationFilePath();
    QStringList arguments;
    arguments << "--mode=local"
              << "--task-id=" + taskId
              << "--bus-url=" + busUrl;

    if (!m_token.isEmpty()) {
        QString tokenFile = "/tmp/gantt_token_" + taskId + ".txt";
        QFile f(tokenFile);
        if (f.open(QIODevice::WriteOnly)) {
            f.write(m_token.toUtf8());
            f.close();
            arguments << "--token-file=" + tokenFile;
        }
    }

    QProcess::startDetached(program, arguments);
}

void ProjectController::initLocalGraphData(const QVariantMap& lgData, const QVariantList& tasks, const QString& busUrl)
{
    qDebug() << "=== initLocalGraphData: tasks=" << tasks.size();
    m_busUrl = busUrl;
    m_projectData->clear();
    m_projectData->set_GraphKind(GanttDefines::GraphKind::Local);
    m_projectData->set_LinkedMasterTaskId(lgData.value("task_id").toString());
    m_projectData->set_ProjectName(lgData.value("title").toString());
    
    QString ts = lgData.value("target_start").toString();
    QString te = lgData.value("target_end").toString();
    if (!ts.isEmpty()) m_projectData->set_LinkedTargetStart(QDate::fromString(ts, Qt::ISODate));
    if (!te.isEmpty()) m_projectData->set_LinkedTargetEnd(QDate::fromString(te, Qt::ISODate));

    QString pe = lgData.value("predecessor_end").toString();
    if (!pe.isEmpty()) m_projectData->set_LinkedPredecessorEndDate(QDate::fromString(pe, Qt::ISODate));

    if (m_projectData->get_groupModel()->rowCount() == 0) {
        m_projectData->get_groupModel()->addGroupWithId("default", "Задачи ЛГ");
    }

    for (const QVariant& tv : tasks) {
        QVariantMap t = tv.toMap();
        QString taskId = t.value("lg_task_id").toString();
        QString title = t.value("title").toString();
        QString responsible = QString::number(t.value("responsible_id").toInt());
        QDate startDate = QDate::fromString(t.value("start_date").toString(), Qt::ISODate);
        QDate endDate = QDate::fromString(t.value("end_date").toString(), Qt::ISODate);
        int status = t.value("status").toInt();

        m_projectData->get_taskModel()->addTaskWithId(
            taskId, "default", title, responsible,
            startDate, endDate, startDate, endDate,
            status, -1, -1, 0
        );
    }

    m_projectData->recalculateEndDate();
    m_projectData->set_Modified(false);
    set_InEditMode(true);
}

void ProjectController::onMastergraphLoaded(const QVariantMap& data)
{
    qDebug() << "=== onMastergraphLoaded: tasks=" << data.value("tasks").toList().size();
    if (!m_projectData->fromJson(data)) {
        emit errorOccurred("Failed to parse mastergraph data");
        return;
    }
    set_InEditMode(true);
    emit projectLoaded();
    emit projectDataChanged();
}

void ProjectController::onLocalGraphLoaded(const QVariantMap& lgData, const QVariantList& tasks)
{
    initLocalGraphData(lgData, tasks, m_busUrl);
    emit projectLoaded();
    emit projectDataChanged();
}

void ProjectController::onHolidaysLoaded(const QVariantList& holidays)
{
    if (m_settingsManager)
        m_settingsManager->setHolidaysFromJson(holidays);
}

void ProjectController::onAccessLoaded(bool canEdit, bool canView, const QString& reason)
{
    Q_UNUSED(canView)
    Q_UNUSED(reason)
    set_InEditMode(canEdit);
}

void ProjectController::onApiError(const QString& message)
{
    qDebug() << "=== onApiError:" << message;
    emit errorOccurred(message);
}

void ProjectController::onWsSnapshot(const QJsonObject& snapshot)
{
    Q_UNUSED(snapshot)
}

void ProjectController::onWsDiff(const QJsonObject& diff)
{
    QJsonArray changes = diff.value("changes").toArray();
    for (const QJsonValue& ch : changes) {
        QJsonObject change = ch.toObject();
        QString op = change.value("op").toString();
        QString entity = change.value("entity").toString();
        QString id = change.value("id").toString();

        if (entity == "task" && op == "update") {
            QJsonObject fields = change.value("fields").toObject();
            QVariantMap task = m_projectData->get_taskModel()->getTask(id);
            if (task.isEmpty()) continue;

            QDate forecastStart = QDate::fromString(fields.value("forecast_start").toString(), Qt::ISODate);
            QDate forecastEnd = QDate::fromString(fields.value("forecast_end").toString(), Qt::ISODate);

            if (forecastStart.isValid() && forecastEnd.isValid())
                m_projectData->get_taskModel()->updateForecastDates(id, forecastStart, forecastEnd);

            int pc = fields.value("progress_current").toInt(-1);
            int pt = fields.value("progress_total").toInt(-1);
            if (pt > 0)
                m_projectData->get_taskModel()->setTaskProgress(id, pc, pt);

            int st = fields.value("status").toInt(-1);
            if (st >= 0)
                m_projectData->get_taskModel()->setTaskStatus(id, static_cast<GanttDefines::TaskStatus>(st));
        }
    }
    emit projectDataChanged();
}

void ProjectController::notifyEditingLocked() { emit errorOccurred("Редактирование заблокировано"); }
void ProjectController::notifyTaskCompleted() { emit errorOccurred("Редактирование завершённых задач запрещено"); }
void ProjectController::notifyMilestoneCompleted() { emit errorOccurred("Перенос пройденной вехи запрещён"); }

void ProjectController::setCursorShape(int shape)
{
    while (QGuiApplication::overrideCursor())
        QGuiApplication::restoreOverrideCursor();

    switch (shape) {
        case 1: QGuiApplication::setOverrideCursor(QCursor(Qt::SizeHorCursor)); break;
        case 2: QGuiApplication::setOverrideCursor(QCursor(Qt::OpenHandCursor)); break;
        default: break;
    }
}

void ProjectController::exportToPng(const QString& filePath)
{
    m_exportManager->exportToPng(m_projectData, filePath);
}

void ProjectController::exportToPdf(const QString& filePath)
{
    m_exportManager->exportToPdf(m_projectData, filePath, QDate::currentDate());
}

void ProjectController::saveProject()
{
    // TODO: отправить накопленные diff в MG Service / BUS через WebSocket
    qDebug() << "=== saveProject: (заглушка, diff отправлять пока некуда)";
}
