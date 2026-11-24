#include "ProjectController.h"
#include <QDebug>
#include <QGuiApplication>
#include <QCursor>
#include <QDate>
#include <QJsonArray>
#include <QJsonObject>
#include <QProcess>
#include <QFile>
#include <QFileInfo>
#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QJsonDocument>
#include <functional>

ProjectController::ProjectController(QObject *parent)
    : QObject(parent), m_projectData(nullptr), m_inEditMode(false)
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

    connect(m_localGraphController, &LocalGraphController::localTasksReloaded, this, &ProjectController::onLocalTasksReloaded);
    connect(m_localGraphController, &LocalGraphController::cascadeUpdateRequested,
            this, &ProjectController::cascadeUpdateRequested);

    connect(m_projectData, &ProjectData::isPureChanged, this, &ProjectController::isPureChanged);
    connect(m_projectData, &ProjectData::isApprovedChanged, this, &ProjectController::isApprovedChanged);
    connect(m_projectData, &ProjectData::graphKindChanged, this, &ProjectController::graphKindChanged);
    connect(m_projectData, &ProjectData::canEditChanged, this, &ProjectController::canEditChanged);

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
    if (m_inEditMode != editMode) { m_inEditMode = editMode; emit inEditModeChanged(); }
}

bool ProjectController::get_isPure() const { return m_projectData->get_isPure(); }
bool ProjectController::get_canEdit() const { return m_projectData->get_canEdit(); }
bool ProjectController::get_isApproved() const { return m_projectData->get_isApproved(); }
bool ProjectController::get_isLocalGraph() const { return m_projectData->get_graphKind() == GanttDefines::GraphKind::Local; }

// ---------------- HTTP ----------------

void ProjectController::_get(const QString& path, std::function<void(int, const QByteArray&)> onDone)
{
    QNetworkAccessManager* mgr = new QNetworkAccessManager(this);
    QNetworkRequest req(QUrl("http://localhost:8080" + path));
    if (!m_token.isEmpty()) req.setRawHeader("Authorization", ("Bearer " + m_token).toUtf8());
    QNetworkReply* reply = mgr->get(req);
    connect(reply, &QNetworkReply::finished, this, [reply, mgr, onDone]() {
        reply->deleteLater(); mgr->deleteLater();
        int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        onDone(status, reply->readAll());
    });
}

void ProjectController::_put(const QString& path, const QJsonObject& body,
                              std::function<void(int, const QByteArray&)> onDone)
{
    QNetworkAccessManager* mgr = new QNetworkAccessManager(this);
    QNetworkRequest req(QUrl("http://localhost:8080" + path));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (!m_token.isEmpty()) req.setRawHeader("Authorization", ("Bearer " + m_token).toUtf8());
    QNetworkReply* reply = mgr->put(req, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [reply, mgr, onDone]() {
        reply->deleteLater(); mgr->deleteLater();
        int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        onDone(status, reply->readAll());
    });
}

void ProjectController::_post(const QString& path, const QJsonObject& body,
                               std::function<void(int, const QByteArray&)> onDone)
{
    QNetworkAccessManager* mgr = new QNetworkAccessManager(this);
    QNetworkRequest req(QUrl("http://localhost:8080" + path));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (!m_token.isEmpty()) req.setRawHeader("Authorization", ("Bearer " + m_token).toUtf8());
    QNetworkReply* reply = mgr->post(req, QJsonDocument(body).toJson());
    connect(reply, &QNetworkReply::finished, this, [reply, mgr, onDone]() {
        reply->deleteLater(); mgr->deleteLater();
        int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        onDone(status, reply->readAll());
    });
}

void ProjectController::_del(const QString& path, std::function<void(int, const QByteArray&)> onDone)
{
    QNetworkAccessManager* mgr = new QNetworkAccessManager(this);
    QNetworkRequest req(QUrl("http://localhost:8080" + path));
    if (!m_token.isEmpty()) req.setRawHeader("Authorization", ("Bearer " + m_token).toUtf8());
    QNetworkReply* reply = mgr->deleteResource(req);
    connect(reply, &QNetworkReply::finished, this, [reply, mgr, onDone]() {
        reply->deleteLater(); mgr->deleteLater();
        int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        onDone(status, reply->readAll());
    });
}

// ---------------- load / open ----------------

void ProjectController::loadFromServer(const QString& mgId, const QString& token)
{
    if (mgId.isEmpty()) { emit errorOccurred("MG ID is empty"); return; }
    m_token = token;
    m_mgId = mgId;
    m_apiClient->setToken(token);
    m_apiClient->loadMastergraph(mgId);
    m_apiClient->loadHolidays();

    // Передаём last_version для WS
    m_webSocket->setLastVersion(m_currentMgVersion);
    m_webSocket->connectToServer(QString("ws://localhost:8080/ws/mastergraphs/%1").arg(mgId), token);

    // Связываем состояние WS с сигналом
    connect(m_webSocket, &WebSocketClient::connectionStateChanged, this, [this]() {
        emit wsConnectionStateChanged(m_webSocket->connectionState());
    });
}

void ProjectController::openForProject(int projectId, bool isReadOnly, const QString& token)
{
    m_projectId = projectId;
    m_token = token;
    m_apiClient->setToken(token);

    _get("/api/v1/mastergraphs?permission=view_project",
         [this, projectId, isReadOnly](int status, const QByteArray& data) {
        if (status != 200) { emit errorOccurred("Не удалось получить список МГ"); return; }
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isArray()) { emit errorOccurred("Некорректный ответ MG"); return; }
        QString mgId;
        for (const QJsonValue& v : doc.array()) {
            QJsonObject obj = v.toObject();
            if (obj.value("project_id").toInt() == projectId) { mgId = obj.value("mg_id").toString(); break; }
        }
        if (mgId.isEmpty()) { emit errorOccurred(QString("МГ для проекта %1 не найден").arg(projectId)); return; }
        m_mgId = mgId;
        loadFromServer(mgId, m_token);
        if (isReadOnly) {
            set_InEditMode(false);
            m_settingsManager->setEditingLocked(true);
            m_projectData->set_CanEdit(false);
        } else {
            m_apiClient->checkAccess(mgId);
        }
    });
}

bool ProjectController::openLocalGraphProcess(const QString& taskId, const QString& busUrl)
{
    if (busUrl.isEmpty()) {
        emit errorOccurred("Не удалось определить BUS для группы задачи");
        return false;
    }
    QString appDir = QCoreApplication::applicationDirPath();
    QStringList candidates;
    #ifdef Q_OS_WIN
    candidates << appDir + "/gantt_editor.exe" << appDir + "/gantt_editor/gantt_editor.exe"
    << appDir + "/../gantt_editor/gantt_editor.exe";
    #else
    candidates << appDir + "/gantt_editor" << appDir + "/gantt_editor/gantt_editor"
    << appDir + "/../gantt_editor/gantt_editor";
    #endif
    QString program;
    for (const QString& c : candidates) {
        QFileInfo fi(c);
        if (fi.exists() && fi.isFile() && fi.isExecutable()) { program = c; break; }
    }
    if (program.isEmpty()) {
        emit errorOccurred("Не найден исполняемый файл gantt_editor");
        return false;
    }
    QStringList arguments;
    arguments << "--mode=local" << "--task-id=" + taskId << "--bus-url=" + busUrl;
    if (!m_token.isEmpty()) {
        QString tokenFile = "/tmp/gantt_token_" + taskId + ".txt";
        QFile f(tokenFile);
        if (f.open(QIODevice::WriteOnly)) { f.write(m_token.toUtf8()); f.close(); arguments << "--token-file=" + tokenFile; }
    }
    qInfo().noquote() << "[MG→LG] launch:"
    << "\n  program:" << program
    << "\n  args:" << arguments.join(" ");
    bool ok = QProcess::startDetached(program, arguments);
    qInfo() << "[MG→LG] started:" << ok;
    if (!ok) emit errorOccurred("Не удалось запустить редактор ЛГ");
    return ok;
}

void ProjectController::initLocalGraphData(const QVariantMap& lgData, const QVariantList& tasks, const QString& busUrl)
{
    m_busUrl = busUrl;
    m_lgTaskId = lgData.value("task_id").toString();
    m_lgProjectId = lgData.value("project_id").toInt();
    m_lgResponsibleId = lgData.value("responsible_id").toInt();
    QString lgId = lgData.value("lg_id").toString();

    m_localGraphController->setLocalGraphContext(lgId, busUrl, m_lgProjectId, m_lgResponsibleId, m_lgTaskId);

    m_projectData->clear();
    m_projectData->set_GraphKind(GanttDefines::GraphKind::Local);
    m_projectData->set_LinkedMasterTaskId(m_lgTaskId);
    m_projectData->set_ProjectName(lgData.value("title").toString());
    m_projectData->set_IsPure(lgData.value("is_pure").toBool());

    bool canEdit = lgData.value("can_edit").toBool();
    m_projectData->set_CanEdit(canEdit);
    m_settingsManager->setEditingLocked(!canEdit);

    qInfo().noquote() << "[initLocalGraphData] can_edit=" << canEdit
    << " is_pure=" << lgData.value("is_pure")
    << " target_start=" << lgData.value("target_start")
    << " target_end=" << lgData.value("target_end")
    << " predecessor_end=" << lgData.value("predecessor_end")
    << " tasks.size=" << tasks.size()
    << " lg_id=" << lgData.value("lg_id");

    QString ts = lgData.value("target_start").toString();
    QString te = lgData.value("target_end").toString();
    if (!ts.isEmpty()) m_projectData->set_LinkedTargetStart(QDate::fromString(ts, Qt::ISODate));
    if (!te.isEmpty()) m_projectData->set_LinkedTargetEnd(QDate::fromString(te, Qt::ISODate));
    QString pe = lgData.value("predecessor_end").toString();
    if (!pe.isEmpty()) m_projectData->set_LinkedPredecessorEndDate(QDate::fromString(pe, Qt::ISODate));

    if (m_projectData->get_groupModel()->rowCount() == 0)
        m_projectData->get_groupModel()->addGroupWithId("default", "Задачи ЛГ");

    for (const QVariant& tv : tasks) {
        QVariantMap t = tv.toMap();
        QString taskId = t.value("lg_task_id").toString();
        QString title = t.value("title").toString();
        int respId = t.value("responsible_id").toInt();
        QString responsible = (respId > 0) ? QString::number(respId) : QString();
        QDate startDate = QDate::fromString(t.value("start_date").toString(), Qt::ISODate);
        QDate endDate = QDate::fromString(t.value("end_date").toString(), Qt::ISODate);
        int status = t.value("status").toInt();
        m_projectData->get_taskModel()->addTaskWithId(
            taskId, "default", title, responsible, startDate, endDate, startDate, endDate,
            status, -1, -1, 0);
    }
    m_projectData->recalculateEndDate();
    m_projectData->set_Modified(false);
    set_InEditMode(true);

    // Подключить WebSocket к BUS для real-time
    if (!lgId.isEmpty() && !busUrl.isEmpty()) {
        QString wsUrl = busUrl;
        wsUrl.replace("http://", "ws://");

        // Обновляем last_version (если есть в ответе)
        int lgVersion = lgData.value("version").toInt(0);
        m_currentMgVersion = lgVersion; // используем общее поле для версии (работает и для ЛГ)

        m_webSocket->setLastVersion(m_currentMgVersion);

        // ВАЖНО: передаём user_id для X-User-ID (для прямого подключения к BUS)
        m_webSocket->setUserId(m_lgResponsibleId);

        m_webSocket->connectToServer(QString("%1/ws/local-graphs/%2").arg(wsUrl, lgId), m_token);

        // Связываем состояние WS с сигналом (для индикатора в UI)
        connect(m_webSocket, &WebSocketClient::connectionStateChanged, this, [this]() {
            emit wsConnectionStateChanged(m_webSocket->connectionState());
        });
    }
}

void ProjectController::onMastergraphLoaded(const QVariantMap& data)
{
    if (!m_projectData->fromJson(data)) {
        emit errorOccurred("Failed to parse mastergraph data");
        return;
    }
    set_InEditMode(true);
    loadProjectMembers();
    emit projectLoaded();
    emit projectDataChanged();
}

void ProjectController::onLocalGraphLoaded(const QVariantMap& lgData, const QVariantList& tasks)
{
    initLocalGraphData(lgData, tasks, m_busUrl);
    emit projectLoaded();
    emit projectDataChanged();
}

void ProjectController::onLocalTasksReloaded()
{
    qDebug() << "[ProjectController] onLocalTasksReloaded: m_lgTaskId=" << m_lgTaskId
    << "m_busUrl=" << m_busUrl;
    if (m_lgTaskId.isEmpty() || m_busUrl.isEmpty()) {
        qDebug() << "[ProjectController] onLocalTasksReloaded: SKIP (empty lgTaskId/busUrl)";
        return;
    }
    m_apiClient->loadLocalGraph(m_lgTaskId, m_busUrl);
}

void ProjectController::onHolidaysLoaded(const QVariantList& holidays)
{
    if (m_settingsManager) m_settingsManager->setHolidaysFromJson(holidays);
}

void ProjectController::onAccessLoaded(bool canEdit, bool canView, const QString& reason)
{
    Q_UNUSED(canView) Q_UNUSED(reason)
    set_InEditMode(canEdit);
    m_settingsManager->setEditingLocked(!canEdit);
    m_projectData->set_CanEdit(canEdit);
}

void ProjectController::onApiError(const QString& message)
{
    emit errorOccurred(message);
}

void ProjectController::onWsSnapshot(const QJsonObject& snapshot)
{
    qDebug() << "ProjectController::onWsSnapshot, tasks:" << snapshot.value("tasks").toArray().size();

    m_currentMgVersion = snapshot.value("version").toInt(m_currentMgVersion);

    if (m_projectData->get_graphKind() == GanttDefines::GraphKind::Local)
    {
        // ЛГ: обновляем задачи + зависимости (группа "default" уже создана)
        QJsonArray tasks = snapshot.value("tasks").toArray();
        m_projectData->get_taskModel()->clear();
        for (const QJsonValue& tv : tasks)
        {
            QJsonObject t = tv.toObject();
            m_projectData->get_taskModel()->addTaskWithId(
                t.value("lg_task_id").toString(), "default",
                t.value("title").toString(),
                QString::number(t.value("responsible_id").toInt()),
                QDate::fromString(t.value("start_date").toString(), Qt::ISODate),
                QDate::fromString(t.value("end_date").toString(), Qt::ISODate),
                QDate::fromString(t.value("start_date").toString(), Qt::ISODate),
                QDate::fromString(t.value("end_date").toString(), Qt::ISODate),
                t.value("status").toInt(), -1, -1, 0);
        }
        // Зависимости ЛГ
        QJsonArray deps = snapshot.value("dependencies").toArray();
        m_projectData->get_dependencyModel()->clear();
        for (const QJsonValue& dv : deps)
        {
            QJsonObject d = dv.toObject();
            m_projectData->get_dependencyModel()->addDependencyWithId(
                d.value("dep_id").toString(),
                d.value("predecessor_id").toString(),
                d.value("successor_id").toString());
        }
        m_projectData->recalculateEndDate();
        qDebug() << "ProjectController: local graph snapshot applied (tasks + deps)";
    } else {
        // МГ: полная загрузка (с группами и вехами)
        m_projectData->fromJson(snapshot.toVariantMap());
        qDebug() << "ProjectController: mastergraph snapshot applied via fromJson";
    }
    emit projectDataChanged();
}

void ProjectController::onWsDiff(const QJsonObject& diff)
{
    QJsonArray changes = diff.value("changes").toArray();
    qDebug() << "[ProjectController] onWsDiff: type=" << diff.value("type").toString()
    << "changes=" << changes.size();
    for (const QJsonValue& ch : changes) {
        QJsonObject change = ch.toObject();
        QString op = change.value("op").toString();
        QString entity = change.value("entity").toString();
        QString id = change.value("id").toString();
        QJsonObject fields = change.value("fields").toObject();

        qDebug() << "[ProjectController] onWsDiff: entity=" << entity
        << "op=" << op << "id=" << id << "fields=" << fields;

        if (entity == "task") {
            if (op == "create" || op == "update") {
                QVariantMap task = m_projectData->get_taskModel()->getTask(id);
                if (!task.isEmpty()) {
                    // forecast_start / forecast_end — применяем НЕЗАВИСИМО.
                    // Если одно из полей отсутствует или null — берём текущее из модели.
                    if (fields.contains("forecast_start") || fields.contains("forecast_end")) {
                        QDate currentFs = task.value("forecastStart").toDate();
                        QDate currentFe = task.value("forecastEnd").toDate();

                        QDate fs = currentFs;
                        QDate fe = currentFe;

                        if (fields.contains("forecast_start")) {
                            QJsonValue v = fields.value("forecast_start");
                            if (!v.isNull() && !v.isUndefined()) {
                                QDate parsed = QDate::fromString(v.toString(), Qt::ISODate);
                                if (parsed.isValid()) fs = parsed;
                            }
                        }
                        if (fields.contains("forecast_end")) {
                            QJsonValue v = fields.value("forecast_end");
                            if (!v.isNull() && !v.isUndefined()) {
                                QDate parsed = QDate::fromString(v.toString(), Qt::ISODate);
                                if (parsed.isValid()) fe = parsed;
                            }
                        }

                        qDebug() << "[ProjectController] onWsDiff: forecast update"
                        << "taskId=" << id
                        << "fs=" << fs << "(current=" << currentFs << ")"
                        << "fe=" << fe << "(current=" << currentFe << ")";

                        if (fs.isValid() && fe.isValid() && (fs != currentFs || fe != currentFe)) {
                            m_projectData->get_taskModel()->updateForecastDates(id, fs, fe);
                        }
                    }
                    if (fields.contains("progress_current") && fields.contains("progress_total")) {
                        int pc = fields.value("progress_current").toInt(-1);
                        int pt = fields.value("progress_total").toInt(-1);
                        if (pt > 0) m_projectData->get_taskModel()->setTaskProgress(id, pc, pt);
                    }
                    if (fields.contains("status")) {
                        int st = fields.value("status").toInt(-1);
                        if (st >= 0) m_projectData->get_taskModel()->setTaskStatus(id, static_cast<GanttDefines::TaskStatus>(st));
                    }
                    if (fields.contains("local_graph_state")) {
                        int lgs = fields.value("local_graph_state").toInt(-1);
                        if (lgs >= 0) m_projectData->get_taskModel()->setLocalGraphState(id, lgs);
                    }
                    if (fields.contains("responsible_id")) {
                        // обновление ответственного — через полную перезагрузку (пока)
                        reloadMastergraph();
                    }
                }
            } else if (op == "delete") {
                m_projectData->get_taskModel()->removeTask(id);
            }
        } else if (entity == "milestone") {
            if (op == "update" || op == "reschedule") {
                QDate planned = QDate::fromString(fields.value("planned_date").toString(), Qt::ISODate);
                if (planned.isValid()) {
                    m_projectData->get_milestoneModel()->rescheduleMilestone(id, planned);
                }
            } else if (op == "complete") {
                m_projectData->get_milestoneModel()->setMilestoneCompleted(id);
            }
        } else if (entity == "dependency") {
            if (op == "create") {
                reloadMastergraph(); // Пока перезагружаем
            } else if (op == "delete") {
                m_projectData->get_dependencyModel()->removeDependency(id);
            }
        } else if (entity == "group") {
            reloadMastergraph(); // Пока перезагружаем
        }
    }
    emit projectDataChanged();
}

void ProjectController::onProjectListLoaded(const QJsonArray& editable, const QJsonArray& viewable)
{
    m_editableProjects = editable;
    m_viewableProjects = viewable;
}

void ProjectController::notifyEditingLocked() { emit errorOccurred("Редактирование заблокировано"); }
void ProjectController::notifyTaskCompleted() { emit errorOccurred("Редактирование завершённых задач запрещено"); }
void ProjectController::notifyMilestoneCompleted() { emit errorOccurred("Перенос пройденной вехи запрещён"); }

void ProjectController::setCursorShape(int shape)
{
    while (QGuiApplication::overrideCursor()) QGuiApplication::restoreOverrideCursor();
    switch (shape) {
        case 1: QGuiApplication::setOverrideCursor(QCursor(Qt::SizeHorCursor)); break;
        case 2: QGuiApplication::setOverrideCursor(QCursor(Qt::OpenHandCursor)); break;
        default: break;
    }
}

void ProjectController::exportToPng(const QString& filePath) { m_exportManager->exportToPng(m_projectData, filePath); }
void ProjectController::exportToPdf(const QString& filePath) { m_exportManager->exportToPdf(m_projectData, filePath, QDate::currentDate()); }

// ---------------- Группы ----------------

void ProjectController::loadDefaultGroups()
{
    if (m_mgId.isEmpty()) { emit errorOccurred("МГ не инициализирован"); return; }
    _get(QString("/api/v1/mastergraphs/%1/default-groups").arg(m_mgId),
         [this](int status, const QByteArray& data) {
        if (status != 200) { emit errorOccurred("Не удалось загрузить список групп"); return; }
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isArray()) { emit errorOccurred("Некорректный ответ"); return; }
        QVariantList list;
        for (const QJsonValue& v : doc.array()) list.append(v.toObject().toVariantMap());
        emit defaultGroupsLoaded(list);
    });
}

void ProjectController::createGroups(const QStringList& groupIds)
{
    if (m_mgId.isEmpty()) { emit groupCreated(false, "МГ не инициализирован"); return; }
    QJsonArray arr;
    for (const QString& g : groupIds) arr.append(g);
    QJsonObject payload; payload["group_ids"] = arr;
    _post(QString("/api/v1/mastergraphs/%1/groups").arg(m_mgId), payload,
          [this](int status, const QByteArray& data) {
              Q_UNUSED(data)
              if (status == 200) {
                  emit groupCreated(true, "Группы созданы");
                  reloadMastergraph(); // <-- ДОБАВЛЕНО: перезагружаем данные, чтобы GridArea перерисовался
              }
              else { emit groupCreated(false, "Ошибка создания групп"); }
          });
}

void ProjectController::createCustomGroup(const QString& name)
{
    if (m_mgId.isEmpty()) { emit groupCreated(false, "МГ не инициализирован"); return; }
    QJsonObject payload; payload["name"] = name;
    _post(QString("/api/v1/mastergraphs/%1/groups/custom").arg(m_mgId), payload,
          [this](int status, const QByteArray& data) {
        Q_UNUSED(data)
        if (status == 200) { emit groupCreated(true, "Группа создана"); reloadMastergraph(); }
        else { emit groupCreated(false, "Ошибка создания группы"); }
    });
}

void ProjectController::deleteGroup(const QString& groupId)
{
    if (m_mgId.isEmpty()) return;
    _del(QString("/api/v1/mastergraphs/%1/groups/%2").arg(m_mgId, groupId),
         [this](int status, const QByteArray& data) {
        Q_UNUSED(data)
        if (status == 204 || status == 200) { emit groupDeleted(true, "Группа удалена"); reloadMastergraph(); }
        else { emit groupDeleted(false, "Ошибка удаления группы"); }
    });
}

// ---------------- Задачи ----------------

void ProjectController::createTask(const QString& groupId, const QString& title,
                                    const QString& responsible, const QDate& startDate, const QDate& endDate)
{
    if (m_mgId.isEmpty()) { emit taskCreated(false, "МГ не инициализирован"); return; }
    QJsonObject payload;
    payload["group_id"] = groupId;
    payload["title"] = title;
    if (!responsible.isEmpty()) { bool ok = false; int rid = responsible.toInt(&ok); if (ok && rid > 0) payload["responsible_id"] = rid; }
    if (startDate.isValid()) payload["start_date"] = startDate.toString(Qt::ISODate);
    if (endDate.isValid()) payload["end_date"] = endDate.toString(Qt::ISODate);
    _post(QString("/api/v1/mastergraphs/%1/tasks").arg(m_mgId), payload,
          [this](int status, const QByteArray& data) {
        if (status == 201) { emit taskCreated(true, "Задача создана"); reloadMastergraph(); }
        else if (status == 409) {
            QJsonDocument doc = QJsonDocument::fromJson(data);
            QVariantList conflicts;
            QJsonObject detail = doc.object().value("detail").toObject();
            for (const QJsonValue& v : detail.value("conflicts").toArray()) conflicts.append(v.toObject().toVariantMap());
            emit titleConflictDetected(conflicts);
            emit taskCreated(false, "Конфликт имён задач");
        } else { emit taskCreated(false, "Ошибка создания задачи"); }
    });
}

void ProjectController::createTaskWithoutDates(const QString& groupId, const QString& title, const QString& responsible)
{
    if (m_mgId.isEmpty()) { emit taskCreated(false, "МГ не инициализирован"); return; }
    QJsonObject payload;
    payload["group_id"] = groupId;
    payload["title"] = title;
    if (!responsible.isEmpty()) { bool ok = false; int rid = responsible.toInt(&ok); if (ok && rid > 0) payload["responsible_id"] = rid; }
    _post(QString("/api/v1/mastergraphs/%1/tasks").arg(m_mgId), payload,
          [this](int status, const QByteArray& data) {
        if (status == 201) { emit taskCreated(true, "Задача создана"); reloadMastergraph(); }
        else if (status == 409) {
            QJsonDocument doc = QJsonDocument::fromJson(data);
            QVariantList conflicts;
            QJsonObject detail = doc.object().value("detail").toObject();
            for (const QJsonValue& v : detail.value("conflicts").toArray()) conflicts.append(v.toObject().toVariantMap());
            emit titleConflictDetected(conflicts);
            emit taskCreated(false, "Конфликт имён задач");
        } else { emit taskCreated(false, "Ошибка создания задачи"); }
    });
}

void ProjectController::createTasksBatch(const QVariantList& tasks, const QVariantList& dependencies)
{
    if (m_mgId.isEmpty()) { emit tasksBatchCreated(false, "МГ не инициализирован"); return; }
    QJsonArray tasksArr, depsArr;
    for (const QVariant& t : tasks) tasksArr.append(QJsonObject::fromVariantMap(t.toMap()));
    for (const QVariant& d : dependencies) depsArr.append(QJsonObject::fromVariantMap(d.toMap()));
    QJsonObject payload;
    payload["tasks"] = tasksArr;
    payload["dependencies"] = depsArr;
    _post(QString("/api/v1/mastergraphs/%1/tasks/batch").arg(m_mgId), payload,
          [this](int status, const QByteArray& data) {
        if (status == 201) { emit tasksBatchCreated(true, "Задачи созданы"); reloadMastergraph(); }
        else if (status == 409) {
            QJsonDocument doc = QJsonDocument::fromJson(data);
            QVariantList conflicts;
            QJsonObject detail = doc.object().value("detail").toObject();
            for (const QJsonValue& v : detail.value("conflicts").toArray()) conflicts.append(v.toObject().toVariantMap());
            emit titleConflictDetected(conflicts);
            emit tasksBatchCreated(false, "Конфликт имён задач");
        } else { emit tasksBatchCreated(false, "Ошибка создания задач"); }
    });
}

void ProjectController::updateTaskDates(const QString& taskId, const QDate& start, const QDate& end,
                                         const QString& reason)
{
    QJsonObject payload;
    payload["start_date"] = start.toString(Qt::ISODate);
    payload["end_date"] = end.toString(Qt::ISODate);
    if (!reason.isEmpty()) payload["reason"] = reason;

    _put(QString("/api/v1/mastergraphs/%1/tasks/%2").arg(m_mgId, taskId), payload,
         [this, taskId, start, end, reason](int status, const QByteArray& data) {
        Q_UNUSED(data)
        if (status == 200) {
            bool addToHistory = m_projectData->get_isApproved();
            m_projectData->get_taskModel()->updateTaskDates(taskId, start, end, addToHistory, reason);
            m_projectData->recalculateEndDate();
            emit projectDataChanged();
            emit taskUpdated(true, "Изменения применены");

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
                if (sStart < end) {
                    int duration = sStart.daysTo(sEnd);
                    QDate newStart = end;
                    QDate newEnd = newStart.addDays(duration);
                    successors.append(QVariantMap{
                        {"taskId", sid},
                        {"title", st.value("title")},
                        {"newStart", newStart.toString(Qt::ISODate)},
                        {"newEnd", newEnd.toString(Qt::ISODate)},
                    });
                }
            }
            if (!successors.isEmpty()) emit cascadeUpdateRequested(successors, reason);
        } else {
            emit taskUpdated(false, "Ошибка применения изменений на стороне сервера");
            reloadMastergraph();
        }
    });
}

void ProjectController::updateTaskForecast(const QString& taskId, const QDate& fs, const QDate& fe,
                                            const QString& reason)
{
    QJsonObject payload;
    if (!reason.isEmpty()) payload["reason"] = reason;
    _put(QString("/api/v1/mastergraphs/%1/tasks/%2").arg(m_mgId, taskId), payload,
         [this, taskId, fs, fe](int status, const QByteArray& data) {
        Q_UNUSED(data)
        if (status == 200) {
            m_projectData->get_taskModel()->updateForecastDates(taskId, fs, fe);
            emit projectDataChanged();
            emit taskUpdated(true, "Прогноз обновлён");
        } else {
            emit taskUpdated(false, "Ошибка обновления прогноза");
            reloadMastergraph();
        }
    });
}

void ProjectController::updateTaskResponsible(const QString& taskId, int responsibleId)
{
    QJsonObject payload;
    payload["responsible_id"] = responsibleId;
    _put(QString("/api/v1/mastergraphs/%1/tasks/%2/responsible").arg(m_mgId, taskId), payload,
         [this](int status, const QByteArray& data) {
        Q_UNUSED(data)
        if (status == 200) { emit taskUpdated(true, "Ответственный назначен"); reloadMastergraph(); }
        else { emit taskUpdated(false, "Ошибка назначения ответственного"); }
    });
}

void ProjectController::updateTaskStatus(const QString& taskId, int status)
{
    QJsonObject payload; payload["status"] = status;
    _put(QString("/api/v1/mastergraphs/%1/tasks/%2").arg(m_mgId, taskId), payload,
         [this](int status_, const QByteArray& data) {
        Q_UNUSED(data)
        if (status_ == 200) { emit taskUpdated(true, "Статус обновлён"); reloadMastergraph(); }
        else { emit taskUpdated(false, "Ошибка обновления статуса"); }
    });
}

void ProjectController::updateTaskProgress(const QString& taskId, int current, int total)
{
    QJsonObject payload;
    payload["progress_current"] = current;
    payload["progress_total"] = total;
    _put(QString("/api/v1/mastergraphs/%1/tasks/%2").arg(m_mgId, taskId), payload,
         [this](int status, const QByteArray& data) {
        Q_UNUSED(data)
        if (status == 200) { emit taskUpdated(true, "Прогресс обновлён"); reloadMastergraph(); }
        else { emit taskUpdated(false, "Ошибка обновления прогресса"); }
    });
}

void ProjectController::updateTaskComment(const QString& taskId, const QString& comment)
{
    QJsonObject payload; payload["comment"] = comment;
    _put(QString("/api/v1/mastergraphs/%1/tasks/%2").arg(m_mgId, taskId), payload,
         [this](int status, const QByteArray& data) {
        Q_UNUSED(data)
        if (status == 200) { emit taskUpdated(true, "Комментарий обновлён"); reloadMastergraph(); }
        else { emit taskUpdated(false, "Ошибка обновления комментария"); }
    });
}

void ProjectController::updateTaskTitle(const QString& taskId, const QString& title)
{
    QJsonObject payload; payload["title"] = title;
    _put(QString("/api/v1/mastergraphs/%1/tasks/%2").arg(m_mgId, taskId), payload,
         [this](int status, const QByteArray& data) {
        Q_UNUSED(data)
        if (status == 200) { emit taskUpdated(true, "Название обновлено"); reloadMastergraph(); }
        else { emit taskUpdated(false, "Ошибка обновления названия"); }
    });
}

void ProjectController::deleteTask(const QString& taskId)
{
    _del(QString("/api/v1/mastergraphs/%1/tasks/%2").arg(m_mgId, taskId),
         [this](int status, const QByteArray& data) {
        Q_UNUSED(data)
        if (status == 204) { emit taskDeleted(true, "Задача удалена"); reloadMastergraph(); }
        else { emit taskDeleted(false, "Ошибка удаления задачи"); }
    });
}

// ---------------- Вехи ----------------

void ProjectController::updateMilestoneDate(const QString& milestoneId, const QDate& newDate, const QString& reason)
{
    QJsonObject payload;
    payload["planned_date"] = newDate.toString(Qt::ISODate);
    if (!reason.isEmpty()) payload["reason"] = reason;
    _put(QString("/api/v1/mastergraphs/%1/milestones/%2").arg(m_mgId, milestoneId), payload,
         [this](int status, const QByteArray& data) {
        Q_UNUSED(data)
        if (status == 200) { emit milestoneUpdated(true, "Веха перенесена"); reloadMastergraph(); }
        else { emit milestoneUpdated(false, "Ошибка переноса вехи"); }
    });
}

void ProjectController::completeMilestone(const QString& milestoneId, const QDate& actualDate)
{
    QJsonObject payload;
    if (actualDate.isValid()) payload["actual_date"] = actualDate.toString(Qt::ISODate);
    _put(QString("/api/v1/mastergraphs/%1/milestones/%2/complete").arg(m_mgId, milestoneId), payload,
         [this](int status, const QByteArray& data) {
        Q_UNUSED(data)
        if (status == 200) { emit milestoneUpdated(true, "Веха пройдена"); reloadMastergraph(); }
        else { emit milestoneUpdated(false, "Ошибка прохождения вехи"); }
    });
}

// ---------------- Статус ----------------

void ProjectController::approveMastergraph(const QString& reason)
{
    QJsonObject payload;
    if (!reason.isEmpty()) payload["reason"] = reason;
    _put(QString("/api/v1/mastergraphs/%1/status").arg(m_mgId), payload,
         [this](int status, const QByteArray& data) {
        if (status == 200) { emit statusChanged(true, "МГ утверждён (Чистовик)"); reloadMastergraph(); }
        else {
            QJsonDocument doc = QJsonDocument::fromJson(data);
            QString msg = doc.object().value("detail").toString("Ошибка смены статуса");
            emit statusChanged(false, msg);
        }
    });
}

// ---------------- Зависимости ----------------

void ProjectController::createDependency(const QString& predId, const QString& succId)
{
    QJsonObject payload;
    payload["predecessor_id"] = predId;
    payload["successor_id"] = succId;
    _post(QString("/api/v1/mastergraphs/%1/dependencies").arg(m_mgId), payload,
          [this](int status, const QByteArray& data) {
        Q_UNUSED(data)
        if (status == 201) { emit dependencyCreated(true, "Зависимость создана"); reloadMastergraph(); }
        else { emit dependencyCreated(false, "Ошибка создания зависимости"); }
    });
}

void ProjectController::deleteDependency(const QString& depId)
{
    _del(QString("/api/v1/mastergraphs/%1/dependencies/%2").arg(m_mgId, depId),
         [this](int status, const QByteArray& data) {
        Q_UNUSED(data)
        if (status == 204) { emit dependencyDeleted(true, "Зависимость удалена"); reloadMastergraph(); }
        else { emit dependencyDeleted(false, "Ошибка удаления зависимости"); }
    });
}

// ---------------- История / участники ----------------

void ProjectController::loadTaskHistory(const QString& taskId)
{
    _get(QString("/api/v1/mastergraphs/%1/history?entity_type=task&entity_id=%2&limit=200").arg(m_mgId, taskId),
         [this, taskId](int status, const QByteArray& data) {
        if (status != 200) { emit errorOccurred("Не удалось загрузить историю"); return; }
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QVariantList events;
        for (const QJsonValue& v : doc.object().value("events").toArray()) events.append(v.toObject().toVariantMap());
        emit taskHistoryLoaded(taskId, events);
    });
}

void ProjectController::loadProjectMembers()
{
    if (m_projectId <= 0) { emit projectMembersChanged(); return; }
    _get(QString("/api/v1/projects/%1").arg(m_projectId),
         [this](int status, const QByteArray& data) {
             if (status != 200) { emit projectMembersChanged(); return; }
             QJsonDocument doc = QJsonDocument::fromJson(data);
             QJsonArray members = doc.object().value("members").toArray();
             QVariantList list;
             for (const QJsonValue& v : members) {
                 QJsonObject m = v.toObject();
                 QString fn = m.value("full_name").toString();
                 if (fn.isEmpty() || fn == "null" || fn == "—") continue;
                 list.append(QVariantMap{
                     {"user_id", m.value("user_id").toInt()},
                             {"full_name", fn},
                             {"role_code", m.value("role_code").toString()},
                 });
             }
             m_projectMembers = list;
             emit projectMembersChanged();
         });
}

// ---------------- Перезагрузка ----------------

void ProjectController::reloadMastergraph()
{
    if (m_mgId.isEmpty()) return;
    m_apiClient->loadMastergraph(m_mgId);
}

void ProjectController::saveProject()
{
    emit saveCompleted(true, "Все изменения уже применены");
}

void ProjectController::renameGroup(const QString& groupId, const QString& newName)
{
    if (m_mgId.isEmpty()) { emit groupCreated(false, "МГ не инициализирован"); return; }
    QJsonObject payload; payload["name"] = newName;
    _put(QString("/api/v1/mastergraphs/%1/groups/%2/rename").arg(m_mgId, groupId), payload,
         [this](int status, const QByteArray& data) {
             Q_UNUSED(data)
             if (status == 200) { emit groupCreated(true, "Группа переименована"); reloadMastergraph(); }
             else { emit groupCreated(false, "Ошибка переименования"); }
         });
}

void ProjectController::createGroupsWithPosition(const QStringList& groupIds, const QString& position, const QString& afterGroupId)
{
    if (m_mgId.isEmpty()) { emit groupCreated(false, "МГ не инициализирован"); return; }
    QJsonArray arr;
    for (const QString& g : groupIds) arr.append(g);
    QJsonObject payload;
    payload["group_ids"] = arr;
    payload["position"] = position;
    if (!afterGroupId.isEmpty()) payload["after_group_id"] = afterGroupId;
    _post(QString("/api/v1/mastergraphs/%1/groups").arg(m_mgId), payload,
          [this](int status, const QByteArray& data)
          {
              Q_UNUSED(data)
              if (status == 200) { emit groupCreated(true, "Группы созданы"); reloadMastergraph(); }
              else { emit groupCreated(false, "Ошибка создания групп"); }
          });
}

void ProjectController::createCustomGroupWithPosition(const QString& name, const QString& position, const QString& afterGroupId)
{
    if (m_mgId.isEmpty()) { emit groupCreated(false, "МГ не инициализирован"); return; }
    QJsonObject payload;
    payload["name"] = name;
    payload["position"] = position;
    if (!afterGroupId.isEmpty()) payload["after_group_id"] = afterGroupId;
    _post(QString("/api/v1/mastergraphs/%1/groups/custom").arg(m_mgId), payload,
          [this](int status, const QByteArray& data)
          {
              Q_UNUSED(data)
              if (status == 200) { emit groupCreated(true, "Группа создана"); reloadMastergraph(); }
              else { emit groupCreated(false, "Ошибка создания группы"); }
          });
}

void ProjectController::updateLocalTaskDates(const QString& taskId, const QDate& start, const QDate& end)
{
    if (m_localGraphController) {
        m_localGraphController->updateLocalTaskDates(taskId, start, end);
    } else {
        qWarning() << "ProjectController::updateLocalTaskDates: localGraphController is null";
        emit errorOccurred("Локальный график не инициализирован");
    }
}
