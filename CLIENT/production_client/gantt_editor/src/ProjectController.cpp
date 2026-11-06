#include "ProjectController.h"
#include <QDebug>
#include <QGuiApplication>
#include <QCursor>
#include <QDate>
#include <QJsonArray>
#include <QJsonObject>

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
    qDebug() << "=== set_InEditMode:" << editMode;
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
    m_apiClient->setToken(token);
    m_apiClient->loadMastergraph(mgId);
    m_apiClient->loadHolidays();

    // WebSocket
    m_webSocket->connectToServer(
        QString("ws://localhost:8080/ws/mastergraphs/%1").arg(mgId),
        token
    );
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

void ProjectController::onHolidaysLoaded(const QVariantList& holidays)
{
    qDebug() << "=== onHolidaysLoaded: count=" << holidays.size();
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
    qDebug() << "=== onWsSnapshot (ignored — REST is source of truth)";
    Q_UNUSED(snapshot)
    // WS snapshot НЕ перезаписывает ProjectData.
    // REST (loadMastergraph) — источник истины с responsible_name / local_graph_state / bus_url.
    // WS используется для diff (onWsDiff — TODO).
}

void ProjectController::onWsDiff(const QJsonObject& diff)
{
    qDebug() << "=== onWsDiff";
    // TODO: применить diff к ProjectData
    QJsonArray changes = diff.value("changes").toArray();
    for (const QJsonValue& ch : changes) {
        QJsonObject change = ch.toObject();
        QString op = change.value("op").toString();
        QString entity = change.value("entity").toString();
        QString id = change.value("id").toString();
        qDebug() << "  diff:" << op << entity << id;
        // TODO: применить к TaskModel / DependencyModel
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
