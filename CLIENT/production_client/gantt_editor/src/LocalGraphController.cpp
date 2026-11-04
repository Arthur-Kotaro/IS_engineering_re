#include "LocalGraphController.h"
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>

LocalGraphController::LocalGraphController(ProjectData* projectData, ResourceManager* resourceManager, SettingsManager* settingsManager, QObject *parent)
    : QObject(parent), m_projectData(projectData), m_resourceManager(resourceManager), m_settingsManager(settingsManager) {}

QString LocalGraphController::localGraphDirectory() const
{
    QString masterPath = m_projectData->get_filePath();
    if (masterPath.isEmpty()) return QString();
    QFileInfo fi(masterPath);
    return fi.absolutePath() + "/subGraph_" + fi.completeBaseName();
}

QString LocalGraphController::getLocalGraphPath(const QString& taskId) const
{
    QString dirPath = localGraphDirectory();
    if (dirPath.isEmpty()) return QString();
    QString masterPath = m_projectData->get_filePath();
    QFileInfo fi(masterPath);
    return dirPath + "/" + taskId + "-" + fi.completeBaseName() + ".gantt";
}

bool LocalGraphController::localGraphFileExists(const QString& taskId) const
{
    QString path = getLocalGraphPath(taskId);
    if (path.isEmpty()) return false;
    return QFile::exists(path);
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
    if (m_projectData->get_filePath().isEmpty()) {
        emit errorOccurred("Сначала сохраните мастерграфик");
        return false;
    }
    QVariantMap task = m_projectData->get_taskModel()->getTask(taskId);
    if (task.isEmpty() || task["status"].toInt() == static_cast<int>(GanttDefines::TaskStatus::Completed)) return false;

    int currentState = task["localGraphState"].toInt();
    if (currentState != static_cast<int>(GanttDefines::LocalGraphState::Required) &&
        currentState != static_cast<int>(GanttDefines::LocalGraphState::Missing)) {
        emit errorOccurred("Сначала пометьте задачу как требующую локальный график");
        return false;
    }

    QString path = getLocalGraphPath(taskId);
    if (path.isEmpty()) return false;
    if (QFile::exists(path)) {
        emit errorOccurred("Файл локального графика уже существует");
        return false;
    }

    QString dirPath = localGraphDirectory();
    QDir dir;
    if (!dir.exists(dirPath) && !dir.mkpath(dirPath)) {
        emit errorOccurred("Не удалось создать каталог: " + dirPath);
        return false;
    }

    QVariantMap localDoc;
    localDoc["projectName"] = task["title"].toString();
    localDoc["projectType"] = "local";
    localDoc["graphKind"] = "local";
    localDoc["linkedMasterTaskId"] = taskId;

    QDate targetStart = task["startDate"].toDate();
    QDate targetEnd = task["endDate"].toDate();
    if (targetStart.isValid() && targetEnd.isValid()) {
        localDoc["linkedTargetStart"] = targetStart.toString("dd.MM.yyyy");
        localDoc["linkedTargetEnd"] = targetEnd.toString("dd.MM.yyyy");
    }

    QStringList predIds = m_projectData->get_dependencyModel()->getPredecessors(taskId);
    QDate maxPredEnd;
    for (const QString& pid : predIds) {
        QVariantMap pt = m_projectData->get_taskModel()->getTask(pid);
        if (pt.isEmpty()) continue;
        QDate peForecast = pt["forecastEnd"].toDate();
        QDate peTarget = pt["endDate"].toDate();
        QDate pe = peForecast.isValid() ? peForecast : peTarget;
        if (pe.isValid() && (!maxPredEnd.isValid() || pe > maxPredEnd)) maxPredEnd = pe;
    }
    if (maxPredEnd.isValid()) localDoc["linkedPredecessorEndDate"] = maxPredEnd.toString("dd.MM.yyyy");

    QDateTime now = QDateTime::currentDateTime();
    localDoc["creationDateTime"] = now.toString("dd.MM.yyyy hh:mm:ss");
    localDoc["lastModifiedDateTime"] = now.toString("dd.MM.yyyy hh:mm:ss");
    localDoc["tasks"] = QVariantList();
    localDoc["dependencies"] = QVariantList();

    if (!m_resourceManager->saveProjectToFile(localDoc, path)) {
        emit errorOccurred("Не удалось сохранить локальный график");
        return false;
    }

    m_projectData->get_taskModel()->setLocalGraphState(taskId, static_cast<int>(GanttDefines::LocalGraphState::Attached));
    m_projectData->set_Modified(true);
    return true;
}

bool LocalGraphController::createLocalGraphOverwrite(const QString& taskId)
{
    // Логика идентична createLocalGraph, но перезаписывает файл, если он существует.
    // Для краткости здесь вызывается удаление старого файла и создание нового.
    QString path = getLocalGraphPath(taskId);
    if (QFile::exists(path)) QFile::remove(path);
    return createLocalGraph(taskId);
}

bool LocalGraphController::attachExistingLocalGraph(const QString& taskId)
{
    if (m_settingsManager->editingLocked()) {
        emit errorOccurred("Редактирование заблокировано");
        return false;
    }
    QString path = getLocalGraphPath(taskId);
    if (path.isEmpty() || !QFile::exists(path)) {
        emit errorOccurred("Файл локального графика не найден");
        return false;
    }
    m_projectData->get_taskModel()->setLocalGraphState(taskId, static_cast<int>(GanttDefines::LocalGraphState::Attached));
    m_projectData->set_Modified(true);
    return true;
}

void LocalGraphController::detachLocalGraph(const QString& taskId, bool deleteFile)
{
    if (m_settingsManager->editingLocked()) {
        emit errorOccurred("Редактирование заблокировано");
        return;
    }
    QVariantMap task = m_projectData->get_taskModel()->getTask(taskId);
    if (task.isEmpty() || task["localGraphState"].toInt() == 0) return;

    QString path = getLocalGraphPath(taskId);
    if (deleteFile && !path.isEmpty() && QFile::exists(path)) {
        if (!QFile::remove(path)) {
            emit errorOccurred("Не удалось удалить файл локального графика");
            return;
        }
    }
    m_projectData->get_taskModel()->setLocalGraphState(taskId, 0);
    m_projectData->set_Modified(true);
}

void LocalGraphController::refreshLocalGraphForecast(const QString& taskId, const QString& localGraphPath, bool markModified)
{
    if (taskId.isEmpty() || localGraphPath.isEmpty()) return;
    QVariantMap task = m_projectData->get_taskModel()->getTask(taskId);
    if (task.isEmpty() || task["status"].toInt() == 1) return;

    QFile file(localGraphPath);
    if (!file.open(QIODevice::ReadOnly)) return;
    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) return;
    QVariantList localTasks = doc.object().toVariantMap()["tasks"].toList();
    if (localTasks.isEmpty()) return;

    QDate minStart, maxEnd;
    int completedCount = 0;
    for (const auto& lt : localTasks) {
        QVariantMap ltm = lt.toMap();
        QDate s = QDate::fromString(ltm["startDate"].toString(), "dd.MM.yyyy");
        QDate e = QDate::fromString(ltm["endDate"].toString(), "dd.MM.yyyy");
        if (s.isValid() && (!minStart.isValid() || s < minStart)) minStart = s;
        if (e.isValid() && (!maxEnd.isValid() || e > maxEnd)) maxEnd = e;
        if (ltm["status"].toInt() == 1) completedCount++;
    }
    if (!minStart.isValid() || !maxEnd.isValid()) return;

    m_projectData->get_taskModel()->updateForecastDates(taskId, minStart, maxEnd);
    m_projectData->get_taskModel()->setTaskProgress(taskId, completedCount, localTasks.size());

    if (completedCount == localTasks.size() && localTasks.size() > 0)
        m_projectData->get_taskModel()->setTaskStatus(taskId, GanttDefines::TaskStatus::Completed);

    m_projectData->recalculateEndDate();
    if (markModified) m_projectData->set_Modified(true);
}

void LocalGraphController::refreshAllLocalGraphForecasts()
{
    if (m_projectData->get_filePath().isEmpty()) return;
    QVariantList allTasks = m_projectData->get_taskModel()->getAllTasks();
    for (const auto& tv : allTasks) {
        QVariantMap task = tv.toMap();
        QString taskId = task["id"].toString();
        int lgs = task["localGraphState"].toInt();
        if (lgs != 2 && lgs != 3) continue; // Attached or Missing
        QString path = getLocalGraphPath(taskId);
        if (path.isEmpty() || !QFile::exists(path)) {
            m_projectData->get_taskModel()->setLocalGraphState(taskId, 3);
            continue;
        }
        refreshLocalGraphForecast(taskId, path, false);
        m_projectData->get_taskModel()->setLocalGraphState(taskId, 2);
    }
}
