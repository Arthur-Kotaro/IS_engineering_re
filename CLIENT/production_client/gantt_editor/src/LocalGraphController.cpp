#include "LocalGraphController.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>

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

void LocalGraphController::setLocalGraphContext(const QString& lgId, const QString& busUrl)
{
    m_lgId = lgId;
    m_busUrl = busUrl;
}

QString LocalGraphController::getBusUrlForTask(const QString& taskId) const
{
    if (!m_busUrl.isEmpty()) return m_busUrl;
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
    payload["responsible_id"] = task["responsible"].toString().toInt();
    payload["title"] = task["title"].toString();
    payload["target_start"] = task["startDate"].toDate().toString(Qt::ISODate);
    payload["target_end"] = task["endDate"].toDate().toString(Qt::ISODate);
    if (predecessorEnd.isValid())
        payload["predecessor_end"] = predecessorEnd.toString(Qt::ISODate);

    m_apiClient->createLocalGraph(taskId, busUrl, payload);
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

// ---- CRUD для задач ЛГ ----

void LocalGraphController::addLocalTask(const QString& title, const QString& responsible, const QDate& startDate, const QDate& endDate)
{
    if (m_lgId.isEmpty() || m_busUrl.isEmpty()) return;

    QVariantMap payload;
    payload["title"] = title;
    payload["responsible_id"] = responsible.toInt();
    if (startDate.isValid()) payload["start_date"] = startDate.toString(Qt::ISODate);
    if (endDate.isValid()) payload["end_date"] = endDate.toString(Qt::ISODate);

    QNetworkAccessManager mgr;
    QNetworkRequest req(QUrl(m_busUrl + "/internal/local-graphs/" + m_lgId + "/tasks"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonDocument doc = QJsonDocument::fromVariant(payload);
    QNetworkReply* reply = mgr.post(req, doc.toJson(QJsonDocument::Compact));
    QObject::connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() == QNetworkReply::NoError) {
            emit localTasksReloaded();
        } else {
            emit errorOccurred("addLocalTask: " + reply->errorString());
        }
    });
}

void LocalGraphController::updateLocalTaskDates(const QString& taskId, const QDate& newStart, const QDate& newEnd)
{
    if (m_lgId.isEmpty() || m_busUrl.isEmpty()) return;

    QVariantMap payload;
    if (newStart.isValid()) payload["start_date"] = newStart.toString(Qt::ISODate);
    if (newEnd.isValid()) payload["end_date"] = newEnd.toString(Qt::ISODate);

    QNetworkAccessManager mgr;
    QNetworkRequest req(QUrl(m_busUrl + "/internal/local-graphs/" + m_lgId + "/tasks/" + taskId));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonDocument doc = QJsonDocument::fromVariant(payload);
    QNetworkReply* reply = mgr.put(req, doc.toJson(QJsonDocument::Compact));
    QObject::connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() == QNetworkReply::NoError) {
            emit localTasksReloaded();
        } else {
            emit errorOccurred("updateLocalTaskDates: " + reply->errorString());
        }
    });
}

void LocalGraphController::setLocalTaskStatus(const QString& taskId, int status)
{
    if (m_lgId.isEmpty() || m_busUrl.isEmpty()) return;

    QVariantMap payload;
    payload["status"] = status;

    QNetworkAccessManager mgr;
    QNetworkRequest req(QUrl(m_busUrl + "/internal/local-graphs/" + m_lgId + "/tasks/" + taskId));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QJsonDocument doc = QJsonDocument::fromVariant(payload);
    QNetworkReply* reply = mgr.put(req, doc.toJson(QJsonDocument::Compact));
    QObject::connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() == QNetworkReply::NoError) {
            emit localTasksReloaded();
        } else {
            emit errorOccurred("setLocalTaskStatus: " + reply->errorString());
        }
    });
}

void LocalGraphController::removeLocalTask(const QString& taskId)
{
    if (m_lgId.isEmpty() || m_busUrl.isEmpty()) return;

    QNetworkAccessManager mgr;
    QNetworkRequest req(QUrl(m_busUrl + "/internal/local-graphs/" + m_lgId + "/tasks/" + taskId));
    QNetworkReply* reply = mgr.deleteResource(req);
    QObject::connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() == QNetworkReply::NoError) {
            emit localTasksReloaded();
        } else {
            emit errorOccurred("removeLocalTask: " + reply->errorString());
        }
    });
}

// ---- Слоты ----

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
