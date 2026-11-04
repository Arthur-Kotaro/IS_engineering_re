#include "TaskController.h"
#include <QDebug>

TaskController::TaskController(ProjectData* projectData, SettingsManager* settingsManager, QObject *parent)
    : QObject(parent), m_projectData(projectData), m_settingsManager(settingsManager) {}

void TaskController::addTask(const QString& groupId, const QString& title, const QString& responsible, const QDate& startDate, const QDate& endDate)
{
    if (m_settingsManager->editingLocked()) {
        emit errorOccurred("Редактирование заблокировано");
        return;
    }

    QDate actualStart = startDate;
    QDate actualEnd = endDate;

    if (m_projectData->get_graphKind() == GanttDefines::GraphKind::Local)
    {
        QDate predEnd = m_projectData->getLinkedPredecessorEndDate();
        if (predEnd.isValid())
        {
            QDate minStart = predEnd.addDays(1);
            if (actualStart.isValid() && actualStart < minStart)
            {
                int duration = actualStart.daysTo(actualEnd);
                actualStart = minStart;
                actualEnd = actualStart.addDays(duration);
                emit errorOccurred("Дата начала скорректирована: не раньше завершения предшественника ("
                                   + predEnd.toString("dd.MM.yyyy") + ")");
            }
        }
    }

    m_projectData->get_taskModel()->addTask(groupId, title, responsible, actualStart, actualEnd);
    m_projectData->recalculateEndDate();
    m_projectData->set_Modified(true);
}

void TaskController::addTaskWithoutDates(const QString& groupId, const QString& title, const QString& responsible)
{
    if (m_settingsManager->editingLocked()) {
        emit errorOccurred("Редактирование заблокировано");
        return;
    }
    m_projectData->get_taskModel()->addTaskWithoutDates(groupId, title, responsible);
    m_projectData->recalculateEndDate();
    m_projectData->set_Modified(true);
}

void TaskController::removeTask(const QString& taskId)
{
    if (m_settingsManager->editingLocked()) {
        emit errorOccurred("Редактирование заблокировано");
        return;
    }

    QVariantMap task = m_projectData->get_taskModel()->getTask(taskId);
    if (task.isEmpty()) return;

    int lgs = task["localGraphState"].toInt();
    if (lgs != static_cast<int>(GanttDefines::LocalGraphState::NotRequired))
    {
        emit confirmRemoveTaskWithLocalGraph(taskId);
        return;
    }

    m_projectData->get_dependencyModel()->removeDependenciesForTask(taskId);
    m_projectData->get_taskModel()->removeTask(taskId);
    m_projectData->recalculateEndDate();
    m_projectData->set_Modified(true);
}

void TaskController::confirmRemoveTask(const QString& taskId)
{
    if (m_settingsManager->editingLocked()) {
        emit errorOccurred("Редактирование заблокировано");
        return;
    }

    m_projectData->get_dependencyModel()->removeDependenciesForTask(taskId);
    m_projectData->get_taskModel()->removeTask(taskId);
    m_projectData->recalculateEndDate();
    m_projectData->set_Modified(true);
}

void TaskController::updateTaskDates(const QString& taskId, const QDate& newStart, const QDate& newEnd)
{
    if (m_settingsManager->editingLocked()) {
        emit errorOccurred("Редактирование заблокировано");
        return;
    }

    QVariantMap task = m_projectData->get_taskModel()->getTask(taskId);
    if (task["status"].toInt() == static_cast<int>(GanttDefines::TaskStatus::Completed)) {
        emit errorOccurred("Нельзя редактировать выполненную задачу");
        return;
    }
    if (newStart > newEnd) {
        emit errorOccurred("Дата завершения не может быть раньше даты начала");
        return;
    }

    if (m_projectData->get_graphKind() == GanttDefines::GraphKind::Local)
    {
        QDate predEnd = m_projectData->getLinkedPredecessorEndDate();
        if (predEnd.isValid())
        {
            QDate minStart = predEnd.addDays(1);
            if (newStart.isValid() && newStart < minStart)
            {
                emit errorOccurred("Дата начала не может быть раньше даты завершения предшественника ("
                                   + predEnd.toString("dd.MM.yyyy") + ")");
                return;
            }
        }
    }

    m_projectData->get_taskModel()->updateTaskDates(taskId, newStart, newEnd, true);
    m_projectData->recalculateEndDate();
    m_projectData->set_Modified(true);

    updateDependentTasks(taskId, newEnd);
}

void TaskController::updateDependentTasks(const QString& taskId, const QDate& newEndDate)
{
    if (m_updatingTasks.contains(taskId)) return;
    m_updatingTasks.insert(taskId);

    QStringList successors = m_projectData->get_dependencyModel()->getSuccessors(taskId);
    for (const QString& successorId : successors)
    {
        QVariantMap successor = m_projectData->get_taskModel()->getTask(successorId);
        QDate successorStart = successor["startDate"].toDate();
        QDate successorEnd = successor["endDate"].toDate();

        if (!successorStart.isValid() || !successorEnd.isValid()) continue;

        int duration = successorStart.daysTo(successorEnd);

        if (successorStart < newEndDate)
        {
            QDate newSuccessorStart = newEndDate;
            QDate newSuccessorEnd = newSuccessorStart.addDays(duration);
            m_projectData->get_taskModel()->updateTaskDates(successorId, newSuccessorStart, newSuccessorEnd, false);
            updateDependentTasks(successorId, newSuccessorEnd);
        }
    }
    m_updatingTasks.remove(taskId);
}

void TaskController::updateForecastDates(const QString& taskId, const QDate& newForecastStart, const QDate& newForecastEnd)
{
    if (m_settingsManager->editingLocked()) {
        emit errorOccurred("Редактирование заблокировано");
        return;
    }

    QVariantMap task = m_projectData->get_taskModel()->getTask(taskId);
    if (task.isEmpty()) return;

    if (task["status"].toInt() == static_cast<int>(GanttDefines::TaskStatus::Completed)) {
        emit errorOccurred("Нельзя редактировать прогноз завершённой задачи");
        return;
    }

    int lgs = task["localGraphState"].toInt();
    if (lgs == static_cast<int>(GanttDefines::LocalGraphState::Attached) ||
        lgs == static_cast<int>(GanttDefines::LocalGraphState::Missing)) {
        emit errorOccurred("Редактирование прогноза заблокировано: задача имеет локальный график");
        return;
    }

    if (newForecastStart > newForecastEnd) {
        emit errorOccurred("Дата завершения прогноза не может быть раньше даты начала");
        return;
    }

    m_projectData->get_taskModel()->updateForecastDates(taskId, newForecastStart, newForecastEnd);
    m_projectData->set_Modified(true);
}

void TaskController::acceptForecastAsTarget(const QString& taskId)
{
    if (m_settingsManager->editingLocked()) {
        emit errorOccurred("Редактирование заблокировано");
        return;
    }
    QVariantMap task = m_projectData->get_taskModel()->getTask(taskId);
    if (task["status"].toInt() == static_cast<int>(GanttDefines::TaskStatus::Completed)) {
        emit errorOccurred("Нельзя утвердить прогноз завершённой задачи");
        return;
    }
    QDate forecastStart = task["forecastStart"].toDate();
    QDate forecastEnd = task["forecastEnd"].toDate();
    QDate targetStart = task["startDate"].toDate();
    QDate targetEnd = task["endDate"].toDate();

    if (!forecastStart.isValid() || !forecastEnd.isValid()) return;
    if (forecastStart == targetStart && forecastEnd == targetEnd) return;

    m_projectData->get_taskModel()->updateTaskDates(taskId, forecastStart, forecastEnd, true);
    m_projectData->recalculateEndDate();
    m_projectData->set_Modified(true);
}

void TaskController::setTaskProgress(const QString& taskId, int current, int total)
{
    if (m_settingsManager->editingLocked()) {
        emit errorOccurred("Редактирование заблокировано");
        return;
    }
    QVariantMap task = m_projectData->get_taskModel()->getTask(taskId);
    if (task.isEmpty()) return;
    if (task["status"].toInt() == static_cast<int>(GanttDefines::TaskStatus::Completed)) {
        emit errorOccurred("Нельзя изменить прогресс завершённой задачи");
        return;
    }
    int lgs = task["localGraphState"].toInt();
    if (lgs == static_cast<int>(GanttDefines::LocalGraphState::Attached) || lgs == static_cast<int>(GanttDefines::LocalGraphState::Missing)) {
        emit errorOccurred("Прогресс задачи с ЛГ транслируется из ЛГ");
        return;
    }
    m_projectData->get_taskModel()->setTaskProgress(taskId, current, total);
    m_projectData->set_Modified(true);
}
