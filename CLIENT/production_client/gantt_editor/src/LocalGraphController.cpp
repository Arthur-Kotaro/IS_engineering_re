#include "LocalGraphController.h"
#include <QDebug>

LocalGraphController::LocalGraphController(ProjectData* projectData,
                                           ResourceManager* resourceManager,
                                           SettingsManager* settingsManager,
                                           GanttApiClient* apiClient,
                                           QObject *parent)
    : QObject(parent)
    , m_projectData(projectData)
    , m_resourceManager(resourceManager)
    , m_settingsManager(settingsManager)
    , m_apiClient(apiClient)
{
    connect(m_apiClient, &GanttApiClient::localGraphCreated, this, &LocalGraphController::onLocalGraphCreated);
    connect(m_apiClient, &GanttApiClient::forecastRecalculated, this, &LocalGraphController::onForecastRecalculated);
}

QString LocalGraphController::getBusUrlForTask(const QString& taskId) const
{
    QVariantMap task = m_projectData->get_taskModel()->getTask(taskId);
    if (task.isEmpty()) return QString();
    QString groupId = task["groupId"].toString();
    return m_projectData->get_groupModel()->getBusUrl(groupId);
}

void LocalGraphController::markTaskRequiresLocalGraph(const QString& taskId)
{
    if (m_settingsManager->editingLocked()) {
        emit errorOccurred("Редактирование заблокировано");
        return;
    }
    QVariantMap task = m_projectData->get_taskModel()->getTask(taskId);
    if (task.isEmpty()) return;
    if (task["status"].toInt() == static_cast<int>(GanttDefines::TaskStatus::Completed)) {
        emit errorOccurred("Нельзя пометить завершённую задачу");
        return;
    }
    int currentState = task["localGraphState"].toInt();
    if (currentState != static_cast<int>(GanttDefines::LocalGraphState::NotRequired)) return;

    m_projectData->get_taskModel()->setLocalGraphState(taskId, static_cast<int>(GanttDefines::LocalGraphState::Required));
    m_projectData->set_Modified(true);
}

bool LocalGraphController::createLocalGraph(const QString& taskId)
{
    if (m_settingsManager->editingLocked()) {
        emit errorOccurred("Редактирование заблокировано");
        return false;
    }
    QVariantMap task = m_projectData->get_taskModel()->getTask(taskId);
    if (task.isEmpty() || task["status"].toInt() == static_cast<int>(GanttDefines::TaskStatus::Completed)) return false;

    QString busUrl = getBusUrlForTask(taskId);
    if (busUrl.isEmpty()) {
        emit errorOccurred("BUS URL не найден для задачи");
        return false;
    }

    // Предшественник
    QDate predecessorEnd;
    QStringList predIds = m_projectData->get_dependencyModel()->getPredecessors(taskId);
    for (const QString& pid : predIds) {
        QVariantMap pt = m_projectData->get_taskModel()->getTask(pid);
        if (pt.isEmpty()) continue;
        QDate peForecast = pt["forecastEnd"].toDate();
        QDate peTarget = pt["endDate"].toDate();
        QDate pe = peForecast.isValid() ? peForecast : peTarget;
        if (pe.isValid() && (!predecessorEnd.isValid() || pe > predecessorEnd))
            predecessorEnd = pe;
    }

    m_pendingTaskId = taskId;

    QVariantMap payload;
    payload["project_id"] = 0;  // TODO: project_id из контекста
    payload["task_id"] = taskId;
    payload["responsible_id"] = task["responsible"].toString().toInt();
    payload["title"] = task["title"].toString();
    payload["target_start"] = task["startDate"].toDate().toString(Qt::ISODate);
    payload["target_end"] = task["endDate"].toDate().toString(Qt::ISODate);
    if (predecessorEnd.isValid())
        payload["predecessor_end"] = predecessorEnd.toString(Qt::ISODate);

    m_apiClient->createLocalGraph(taskId, busUrl, payload);
    return true;
}

bool LocalGraphController::createLocalGraphOverwrite(const QString& taskId)
{
    return createLocalGraph(taskId);
}

bool LocalGraphController::attachExistingLocalGraph(const QString& taskId)
{
    if (m_settingsManager->editingLocked()) {
        emit errorOccurred("Редактирование заблокировано");
        return false;
    }
    m_projectData->get_taskModel()->setLocalGraphState(taskId, static_cast<int>(GanttDefines::LocalGraphState::Attached));
    m_projectData->set_Modified(true);
    return true;
}

void LocalGraphController::detachLocalGraph(const QString& taskId, bool deleteFile)
{
    Q_UNUSED(deleteFile)
    if (m_settingsManager->editingLocked()) {
        emit errorOccurred("Редактирование заблокировано");
        return;
    }
    QVariantMap task = m_projectData->get_taskModel()->getTask(taskId);
    if (task.isEmpty() || task["localGraphState"].toInt() == 0) return;

    m_projectData->get_taskModel()->setLocalGraphState(taskId, 0);
    m_projectData->set_Modified(true);
    // TODO: DELETE /internal/local-graphs/{lg_id} в BUS (позже)
}

void LocalGraphController::refreshLocalGraphForecast(const QString& taskId)
{
    QVariantMap task = m_projectData->get_taskModel()->getTask(taskId);
    if (task.isEmpty() || task["status"].toInt() == 1) return;

    QString busUrl = getBusUrlForTask(taskId);
    if (busUrl.isEmpty()) return;

    QDate predecessorEnd;
    QStringList predIds = m_projectData->get_dependencyModel()->getPredecessors(taskId);
    for (const QString& pid : predIds) {
        QVariantMap pt = m_projectData->get_taskModel()->getTask(pid);
        if (pt.isEmpty()) continue;
        QDate peForecast = pt["forecastEnd"].toDate();
        QDate peTarget = pt["endDate"].toDate();
        QDate pe = peForecast.isValid() ? peForecast : peTarget;
        if (pe.isValid() && (!predecessorEnd.isValid() || pe > predecessorEnd))
            predecessorEnd = pe;
    }

    m_pendingTaskId = taskId;

    QVariantMap payload;
    payload["project_id"] = 0;
    payload["task_id"] = taskId;
    if (predecessorEnd.isValid())
        payload["predecessor_end"] = predecessorEnd.toString(Qt::ISODate);

    m_apiClient->recalcForecast(taskId, busUrl, payload);
}

void LocalGraphController::onLocalGraphCreated(const QVariantMap& result)
{
    qDebug() << "=== onLocalGraphCreated:" << result.value("lg_id");
    if (m_pendingTaskId.isEmpty()) return;
    m_projectData->get_taskModel()->setLocalGraphState(
        m_pendingTaskId, static_cast<int>(GanttDefines::LocalGraphState::Attached));
    emit localGraphCreated(m_pendingTaskId);
    m_pendingTaskId.clear();
}

void LocalGraphController::onForecastRecalculated(const QVariantMap& result)
{
    qDebug() << "=== onForecastRecalculated";
    if (m_pendingTaskId.isEmpty()) return;

    QDate start = QDate::fromString(result.value("forecast_start").toString(), Qt::ISODate);
    QDate end = QDate::fromString(result.value("forecast_end").toString(), Qt::ISODate);
    int progressCurrent = result.value("progress_current").toInt();
    int progressTotal = result.value("progress_total").toInt();
    int status = result.value("status").toInt();

    if (start.isValid() && end.isValid())
        m_projectData->get_taskModel()->updateForecastDates(m_pendingTaskId, start, end);

    if (progressTotal > 0)
        m_projectData->get_taskModel()->setTaskProgress(m_pendingTaskId, progressCurrent, progressTotal);

    if (status == 1)
        m_projectData->get_taskModel()->setTaskStatus(m_pendingTaskId, GanttDefines::TaskStatus::Completed);

    m_projectData->recalculateEndDate();
    m_projectData->set_Modified(true);
    emit localGraphRefreshed(m_pendingTaskId);
    m_pendingTaskId.clear();
}
