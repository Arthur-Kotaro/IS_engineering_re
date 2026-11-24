#include "LocalGraphController.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
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
    , m_networkManager(new QNetworkAccessManager(this))
{
    connect(m_apiClient, &GanttApiClient::localGraphCreated, this, &LocalGraphController::onLocalGraphCreated);
    connect(m_apiClient, &GanttApiClient::forecastRecalculated, this, &LocalGraphController::onForecastRecalculated);
    connect(m_apiClient, &GanttApiClient::lgDependencyCreated, this, &LocalGraphController::onLgDependencyCreated);
    connect(m_apiClient, &GanttApiClient::lgDependencyDeleted, this, &LocalGraphController::onLgDependencyDeleted);
    connect(m_apiClient, &GanttApiClient::lgDependenciesLoaded, this, &LocalGraphController::onLgDependenciesLoaded);
}

void LocalGraphController::setLocalGraphContext(const QString& lgId, const QString& busUrl,
                                                 int projectId, int responsibleId, const QString& masterTaskId)
{
    m_lgId = lgId;
    m_busUrl = busUrl;
    m_projectId = projectId;
    m_responsibleId = responsibleId;
    m_masterTaskId = masterTaskId;
    // BUS-запросы должны ставить X-User-ID — передаём userId в GanttApiClient
    m_apiClient->setUserId(m_responsibleId);
    qDebug() << "LocalGraphController::setLocalGraphContext: lgId=" << lgId
             << "busUrl=" << busUrl << "projectId=" << projectId
             << "responsibleId=" << responsibleId << "masterTaskId=" << masterTaskId;
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
    payload["project_id"] = m_projectId;
    payload["task_id"] = taskId;
    payload["responsible_id"] = m_responsibleId;
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
    payload["project_id"] = m_projectId;
    payload["task_id"] = taskId;
    if (predecessorEnd.isValid())
        payload["predecessor_end"] = predecessorEnd.toString(Qt::ISODate);

    m_apiClient->recalcForecast(taskId, busUrl, payload);
}

// ============================================================
// CRUD для задач ЛГ.
// Единый принцип: клиент шлёт команду, сервер применяет,
// BUS рассылает WS snapshot, клиент перерисовывает из snapshot.
// НИКАКИХ локальных изменений TaskModel/DependencyModel,
// НИКАКИХ emit localTasksReloaded/localDependenciesReloaded.
// ============================================================

void LocalGraphController::addLocalTask(const QString& title, const QString& responsible,
                                         const QDate& startDate, const QDate& endDate)
{
    qDebug() << "LocalGraphController::addLocalTask title=" << title
             << "lgId=" << m_lgId << "busUrl=" << m_busUrl << "responsibleId=" << m_responsibleId;
    if (m_lgId.isEmpty() || m_busUrl.isEmpty()) {
        emit errorOccurred("Локальный график не инициализирован");
        return;
    }

    QVariantMap payload;
    payload["title"] = title;
    payload["responsible_id"] = responsible.toInt();
    if (startDate.isValid()) payload["start_date"] = startDate.toString(Qt::ISODate);
    if (endDate.isValid()) payload["end_date"] = endDate.toString(Qt::ISODate);

    QNetworkRequest req(QUrl(m_busUrl + "/internal/local-graphs/" + m_lgId + "/tasks"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("X-User-ID", QByteArray::number(m_responsibleId));

    QJsonDocument doc = QJsonDocument::fromVariant(payload);
    QNetworkReply* reply = m_networkManager->post(req, doc.toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QByteArray body = reply->readAll();
        qDebug() << "[LocalGraphController] addLocalTask FINISHED status=" << status
                 << "error=" << reply->error() << "body=" << body;
        if (reply->error() != QNetworkReply::NoError) {
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

    QNetworkRequest req(QUrl(m_busUrl + "/internal/local-graphs/" + m_lgId + "/tasks/" + taskId));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("X-User-ID", QByteArray::number(m_responsibleId));

    QJsonDocument doc = QJsonDocument::fromVariant(payload);
    QNetworkReply* reply = m_networkManager->put(req, doc.toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply, taskId, newStart, newEnd]() {
        reply->deleteLater();
        int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << "[LocalGraphController] updateLocalTaskDates FINISHED status=" << status
                 << "taskId=" << taskId << "newStart=" << newStart << "newEnd=" << newEnd;
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred("updateLocalTaskDates: " + reply->errorString());
            return;
        }
        // Каскад — клиентская подсказка пользователю (до WS snapshot).
        // TODO: перенести каскад на сервер (в BUS), чтобы клиент не дублировал логику.
        QVariantList successors;
        auto* depModel = m_projectData->get_dependencyModel();
        auto* taskModel = m_projectData->get_taskModel();
        QStringList succIds = depModel->getSuccessors(taskId);
        for (const QString& sid : succIds) {
            QVariantMap st = taskModel->getTask(sid);
            if (st.isEmpty()) continue;
            QDate sStart = st.value("startDate").toDate();
            QDate sEnd = st.value("endDate").toDate();
            if (!sStart.isValid() || !sEnd.isValid()) continue;
            if (sStart < newEnd) {
                int duration = sStart.daysTo(sEnd);
                QDate newStart2 = newEnd;
                QDate newEnd2 = newStart2.addDays(duration);
                successors.append(QVariantMap{
                    {"taskId", sid},
                    {"title", st.value("title")},
                    {"newStart", newStart2.toString(Qt::ISODate)},
                    {"newEnd", newEnd2.toString(Qt::ISODate)},
                });
            }
        }
        if (!successors.isEmpty()) emit cascadeUpdateRequested(successors, "");
    });
}

void LocalGraphController::setLocalTaskStatus(const QString& taskId, int status)
{
    if (m_lgId.isEmpty() || m_busUrl.isEmpty()) return;

    QVariantMap payload;
    payload["status"] = status;

    QNetworkRequest req(QUrl(m_busUrl + "/internal/local-graphs/" + m_lgId + "/tasks/" + taskId));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("X-User-ID", QByteArray::number(m_responsibleId));

    QJsonDocument doc = QJsonDocument::fromVariant(payload);
    QNetworkReply* reply = m_networkManager->put(req, doc.toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply, taskId, status]() {
        reply->deleteLater();
        int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << "[LocalGraphController] setLocalTaskStatus FINISHED status=" << httpStatus
                 << "taskId=" << taskId << "newStatus=" << status;
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred("setLocalTaskStatus: " + reply->errorString());
        }
    });
}

void LocalGraphController::removeLocalTask(const QString& taskId)
{
    if (m_lgId.isEmpty() || m_busUrl.isEmpty()) return;

    QNetworkRequest req(QUrl(m_busUrl + "/internal/local-graphs/" + m_lgId + "/tasks/" + taskId));
    req.setRawHeader("X-User-ID", QByteArray::number(m_responsibleId));

    QNetworkReply* reply = m_networkManager->deleteResource(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, taskId]() {
        reply->deleteLater();
        int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << "[LocalGraphController] removeLocalTask FINISHED status=" << status
                 << "taskId=" << taskId;
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred("removeLocalTask: " + reply->errorString());
        }
    });
}

// ============================================================
// Зависимости ЛГ. Тот же принцип.
// ============================================================

void LocalGraphController::loadLocalDependencies()
{
    if (m_lgId.isEmpty() || m_busUrl.isEmpty()) return;
    m_apiClient->loadLgDependencies(m_lgId, m_busUrl);
}

void LocalGraphController::addLocalDependency(const QString& predecessorId, const QString& successorId)
{
    qDebug() << "[LocalGraphController] addLocalDependency pred=" << predecessorId
             << "succ=" << successorId << "lgId=" << m_lgId << "responsibleId=" << m_responsibleId;
    if (m_lgId.isEmpty() || m_busUrl.isEmpty()) {
        emit errorOccurred("Локальный график не инициализирован");
        return;
    }
    m_apiClient->createLgDependency(m_lgId, m_busUrl, predecessorId, successorId);
}

void LocalGraphController::removeLocalDependency(const QString& depId)
{
    if (m_lgId.isEmpty() || m_busUrl.isEmpty()) {
        emit errorOccurred("Локальный график не инициализирован");
        return;
    }
    m_apiClient->deleteLgDependency(m_lgId, m_busUrl, depId);
}

// ============================================================
// Слоты (ответы на команды). НЕ меняют модели напрямую —
// модели меняются только из WS snapshot (onWsSnapshot).
// ============================================================

void LocalGraphController::onLocalGraphCreated(const QVariantMap& result)
{
    qDebug() << "=== onLocalGraphCreated:" << result.value("lg_id");
    if (m_pendingTaskId.isEmpty()) return;
    // local_graph_state обновится из следующего snapshot (WS или reloadMastergraph).
    emit localGraphCreated(m_pendingTaskId);
    m_pendingTaskId.clear();
}

void LocalGraphController::onForecastRecalculated(const QVariantMap& result)
{
    qDebug() << "=== onForecastRecalculated";
    if (m_pendingTaskId.isEmpty()) return;
    // Прогноз обновится из WS snapshot.
    emit localGraphRefreshed(m_pendingTaskId);
    m_pendingTaskId.clear();
}

void LocalGraphController::onLgDependencyCreated(const QVariantMap& result)
{
    qDebug() << "=== onLgDependencyCreated:" << result.value("dep_id");
    // Зависимость придёт из WS snapshot. Ничего не делаем.
}

void LocalGraphController::onLgDependencyDeleted(const QString& depId)
{
    qDebug() << "=== onLgDependencyDeleted:" << depId;
    // Удаление придёт из WS snapshot. Ничего не делаем.
}

void LocalGraphController::onLgDependenciesLoaded(const QVariantList& deps)
{
    qDebug() << "=== onLgDependenciesLoaded: count=" << deps.size();
    // Полная перезагрузка зависимостей — используется ТОЛЬКО при явном
    // loadLocalDependencies() (например, из onWsSnapshot, если snapshot
    // не содержит dependencies, или по кнопке «Обновить»).
    // Заполняет DependencyModel из списка сервера (серверный dep_id).
    auto* depModel = m_projectData->get_dependencyModel();
    depModel->clear();
    for (const QVariant& dv : deps) {
        QVariantMap d = dv.toMap();
        depModel->addDependencyWithId(
            d.value("dep_id").toString(),
            d.value("predecessor_id").toString(),
            d.value("successor_id").toString());
    }
}
