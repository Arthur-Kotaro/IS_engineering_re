#include "ProjectController.h"
#include <QDebug>
#include <QGuiApplication>
#include <QCursor>
#include <QDate>

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

    m_taskController = new TaskController(m_projectData, m_settingsManager, this);
    m_localGraphController = new LocalGraphController(m_projectData, m_resourceManager, m_settingsManager, this);
    m_dependencyController = new DependencyController(m_projectData, m_settingsManager, this);

    m_resourceManager->setResourcesPath(m_settingsManager->resourcesPath());

    // Проброс сигналов ошибок
    connect(m_taskController, &TaskController::errorOccurred, this, &ProjectController::errorOccurred);
    connect(m_localGraphController, &LocalGraphController::errorOccurred, this, &ProjectController::errorOccurred);
    connect(m_dependencyController, &DependencyController::errorOccurred, this, &ProjectController::errorOccurred);
    connect(m_taskController, &TaskController::confirmRemoveTaskWithLocalGraph, this, &ProjectController::confirmRemoveTaskWithLocalGraph);

    // GanttApiClient
    connect(m_apiClient, &GanttApiClient::mastergraphLoaded, this, &ProjectController::onMastergraphLoaded);
    connect(m_apiClient, &GanttApiClient::accessLoaded, this, &ProjectController::onAccessLoaded);
    connect(m_apiClient, &GanttApiClient::holidaysLoaded, this, &ProjectController::onHolidaysLoaded);
    connect(m_apiClient, &GanttApiClient::errorOccurred, this, &ProjectController::onApiError);

    m_apiClient->setBaseUrl("http://localhost:8080");
}

ProjectData* ProjectController::get_projectData() const { return m_projectData; }
ResourceManager* ProjectController::get_resourceManager() const { return m_resourceManager; }
SettingsManager* ProjectController::get_settingsManager() const { return m_settingsManager; }
ExportManager* ProjectController::get_exportManager() const { return m_exportManager; }
TaskController* ProjectController::get_taskController() const { return m_taskController; }
LocalGraphController* ProjectController::get_localGraphController() const { return m_localGraphController; }
DependencyController* ProjectController::get_dependencyController() const { return m_dependencyController; }

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
    qDebug() << "=== loadFromServer called: mgId=" << mgId << "token length=" << token.length();
    if (mgId.isEmpty()) {
        emit errorOccurred("MG ID is empty");
        return;
    }
    m_apiClient->setToken(token);
    m_apiClient->loadMastergraph(mgId);
    m_apiClient->loadHolidays();
}

void ProjectController::onMastergraphLoaded(const QVariantMap& data)
{
    qDebug() << "=== onMastergraphLoaded: tasks=" << data.value("tasks").toList().size()
             << "groups=" << data.value("groups").toList().size();
    if (!m_projectData->fromJson(data)) {
        qDebug() << "=== fromJson FAILED";
        emit errorOccurred("Failed to parse mastergraph data");
        return;
    }
    qDebug() << "=== fromJson OK, setting inEditMode=true";
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
