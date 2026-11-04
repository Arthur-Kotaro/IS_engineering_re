#include "TaskModel.h"
#include <QUuid>
#include <QDebug>

TaskModel::TaskModel(QObject *parent) : QAbstractListModel(parent) {}

int TaskModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_tasks.count();
}

QHash<int, QByteArray> TaskModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[GanttDefines::IdRole] = "taskId";
    roles[GanttDefines::TitleRole] = "title";
    roles[GanttDefines::ResponsibleRole] = "responsible";
    roles[GanttDefines::StartDateRole] = "startDate";
    roles[GanttDefines::EndDateRole] = "endDate";
    roles[GanttDefines::StatusRole] = "status";
    roles[GanttDefines::GroupIdRole] = "groupId";
    roles[GanttDefines::CommentRole] = "comment";
    roles[GanttDefines::ForecastStartRole] = "forecastStart";
    roles[GanttDefines::ForecastEndRole] = "forecastEnd";
    roles[GanttDefines::ProgressCurrentRole] = "progressCurrent";
    roles[GanttDefines::ProgressTotalRole] = "progressTotal";
    roles[GanttDefines::LocalGraphStateRole] = "localGraphState";
    return roles;
}

QVariant TaskModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_tasks.size()) return QVariant();

    const Task &task = m_tasks[index.row()];
    switch (role)
    {
        case GanttDefines::IdRole: return task.id;
        case GanttDefines::TitleRole: return task.title;
        case GanttDefines::ResponsibleRole: return task.responsible;
        case GanttDefines::StartDateRole: return task.startDate;
        case GanttDefines::EndDateRole: return task.endDate;
        case GanttDefines::StatusRole: return static_cast<int>(task.status);
        case GanttDefines::GroupIdRole: return task.groupId;
        case GanttDefines::CommentRole: return task.comment;
        case GanttDefines::ForecastStartRole: return task.forecastStart;
        case GanttDefines::ForecastEndRole: return task.forecastEnd;
        case GanttDefines::ProgressCurrentRole: return task.progressCurrent;
        case GanttDefines::ProgressTotalRole: return task.progressTotal;
        case GanttDefines::LocalGraphStateRole: return static_cast<int>(task.localGraphState);
        default: return QVariant();
    }
}

QString TaskModel::generateId() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

int TaskModel::findTaskIndex(const QString& taskId) const
{
    for (int i = 0; i < m_tasks.size(); ++i)
    {
        if (m_tasks[i].id == taskId) return i;
    }
    return -1;
}

void TaskModel::addTask(const QString& groupId, const QString& title, const QString& responsible,
                        const QDate& startDate, const QDate& endDate)
{
    beginInsertRows(QModelIndex(), m_tasks.size(), m_tasks.size());
    Task task;
    task.id = generateId();
    task.title = title;
    task.responsible = responsible;
    task.startDate = startDate;
    task.endDate = endDate;
    task.forecastStart = startDate;
    task.forecastEnd = endDate;
    task.status = GanttDefines::TaskStatus::Planned;
    task.groupId = groupId;
    m_tasks.append(task);
    endInsertRows();
    emit countChanged();
}

void TaskModel::addTaskWithoutDates(const QString& groupId, const QString& title, const QString& responsible)
{
    beginInsertRows(QModelIndex(), m_tasks.size(), m_tasks.size());
    Task task;
    task.id = generateId();
    task.title = title;
    task.responsible = responsible;
    task.startDate = QDate();
    task.endDate = QDate();
    task.forecastStart = QDate();
    task.forecastEnd = QDate();
    task.status = GanttDefines::TaskStatus::Planned;
    task.groupId = groupId;
    m_tasks.append(task);
    endInsertRows();
    emit countChanged();
}

void TaskModel::addTaskWithId(const QString& taskId, const QString& groupId, const QString& title,
                              const QString& responsible, const QDate& startDate, const QDate& endDate,
                              const QDate& forecastStart, const QDate& forecastEnd, int status,
                              int progressCurrent, int progressTotal, int localGraphState)
{
    beginInsertRows(QModelIndex(), m_tasks.size(), m_tasks.size());
    Task task;
    task.id = taskId;
    task.title = title;
    task.responsible = responsible;
    task.startDate = startDate;
    task.endDate = endDate;
    task.status = static_cast<GanttDefines::TaskStatus>(status);

    if (task.status == GanttDefines::TaskStatus::Completed)
    {
        task.forecastStart = startDate;
        task.forecastEnd = endDate;
    }
    else
    {
        task.forecastStart = forecastStart.isValid() ? forecastStart : startDate;
        task.forecastEnd = forecastEnd.isValid() ? forecastEnd : endDate;
    }

    task.groupId = groupId;

    if (progressCurrent >= 0 && progressTotal > 0)
    {
        task.progressCurrent = progressCurrent;
        task.progressTotal = progressTotal;
    }

    if (localGraphState >= 0 && localGraphState <= 3)
        task.localGraphState = static_cast<GanttDefines::LocalGraphState>(localGraphState);

    m_tasks.append(task);
    endInsertRows();
    emit countChanged();
}

void TaskModel::removeTask(const QString& taskId)
{
    int index = findTaskIndex(taskId);
    if (index >= 0)
    {
        beginRemoveRows(QModelIndex(), index, index);
        for (auto* entry : m_tasks[index].history) delete entry;
        m_tasks.removeAt(index);
        endRemoveRows();
        emit countChanged();
    }
}

void TaskModel::updateTask(const QString& taskId, const QString& title, const QString& responsible,
                           const QDate& startDate, const QDate& endDate, int status)
{
    int index = findTaskIndex(taskId);
    if (index >= 0)
    {
        Task& task = m_tasks[index];
        task.title = title;
        task.responsible = responsible;
        task.startDate = startDate;
        task.endDate = endDate;
        task.status = static_cast<GanttDefines::TaskStatus>(status);

        if (task.status == GanttDefines::TaskStatus::Completed)
        {
            task.forecastStart = startDate;
            task.forecastEnd = endDate;
        }

        QModelIndex modelIndex = createIndex(index, 0);
        emit dataChanged(modelIndex, modelIndex);
    }
}

void TaskModel::updateTaskDates(const QString& taskId, const QDate& newStart, const QDate& newEnd, bool addToHistory)
{
    int index = findTaskIndex(taskId);
    if (index >= 0 && newStart <= newEnd)
    {
        Task& task = m_tasks[index];

        // П.7: Запоминаем, были ли у задачи сроки ДО изменения (чтобы не сохранять "пустой" след в историю)
        bool hadDates = task.startDate.isValid() && task.endDate.isValid();

        if (addToHistory && hadDates)
        {
            QPair<QDate, QDate> oldDates(task.startDate, task.endDate);
            task.dateHistory.append(oldDates);
        }

        task.startDate = newStart;
        task.endDate = newEnd;

        // П.6: Если у задачи не было сроков (это первая установка), или она завершена —
        // прогноз должен стать равным целевому.
        if (!hadDates || task.status == GanttDefines::TaskStatus::Completed)
        {
            task.forecastStart = newStart;
            task.forecastEnd = newEnd;
        }

        QModelIndex modelIndex = createIndex(index, 0);
        emit dataChanged(modelIndex, modelIndex);
        emit taskDatesChanged(taskId);

        // Эмитим сигнал прогноза, если он изменился (для завершённых или при первой установке)
        if (!hadDates || task.status == GanttDefines::TaskStatus::Completed)
        {
            emit taskForecastDatesChanged(taskId);
        }
    }
}

void TaskModel::updateForecastDates(const QString& taskId, const QDate& newForecastStart, const QDate& newForecastEnd)
{
    int index = findTaskIndex(taskId);
    if (index >= 0 && newForecastStart.isValid() && newForecastEnd.isValid() && newForecastStart <= newForecastEnd)
    {
        Task& task = m_tasks[index];
        if (task.status == GanttDefines::TaskStatus::Completed) return;

        task.forecastStart = newForecastStart;
        task.forecastEnd = newForecastEnd;

        QModelIndex modelIndex = createIndex(index, 0);
        emit dataChanged(modelIndex, modelIndex, {GanttDefines::ForecastStartRole, GanttDefines::ForecastEndRole});
        emit taskForecastDatesChanged(taskId);
    }
}

void TaskModel::setTaskStatus(const QString& taskId, GanttDefines::TaskStatus status)
{
    int index = findTaskIndex(taskId);
    if (index >= 0)
    {
        Task& task = m_tasks[index];
        task.status = status;

        if (task.status == GanttDefines::TaskStatus::Completed)
        {
            task.forecastStart = task.startDate;
            task.forecastEnd = task.endDate;
        }

        QModelIndex modelIndex = createIndex(index, 0);
        emit dataChanged(modelIndex, modelIndex);
    }
}

void TaskModel::setTaskComment(const QString& taskId, const QString& comment)
{
    int index = findTaskIndex(taskId);
    if (index >= 0)
    {
        m_tasks[index].comment = comment;
        QModelIndex modelIndex = createIndex(index, 0);
        emit dataChanged(modelIndex, modelIndex, {GanttDefines::CommentRole});
    }
}

void TaskModel::setTaskProgress(const QString& taskId, int current, int total)
{
    int index = findTaskIndex(taskId);
    if (index < 0) return;

    Task& task = m_tasks[index];

    bool progressValid = (current >= 0 && total > 0 && current <= total);

    if (!progressValid)
    {
        task.progressCurrent = -1;
        task.progressTotal = -1;
    }
    else
    {
        task.progressCurrent = current;
        task.progressTotal = total;
    }

    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex, {GanttDefines::ProgressCurrentRole, GanttDefines::ProgressTotalRole});
    emit taskProgressChanged(taskId);

    if (progressValid
        && task.progressCurrent == task.progressTotal
        && task.status != GanttDefines::TaskStatus::Completed)
    {
        setTaskStatus(taskId, GanttDefines::TaskStatus::Completed);
    }
}

void TaskModel::setLocalGraphState(const QString& taskId, int state)
{
    int index = findTaskIndex(taskId);
    if (index < 0) return;
    if (state < 0 || state > 3) return;

    m_tasks[index].localGraphState = static_cast<GanttDefines::LocalGraphState>(state);
    QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex, {GanttDefines::LocalGraphStateRole});
    emit taskLocalGraphStateChanged(taskId);
}

void TaskModel::addDateHistory(const QString& taskId, const QDate& oldStart, const QDate& oldEnd)
{
    int index = findTaskIndex(taskId);
    if (index >= 0)
    {
        QPair<QDate, QDate> oldDates(oldStart, oldEnd);
        m_tasks[index].dateHistory.append(oldDates);
        QModelIndex modelIndex = createIndex(index, 0);
        emit dataChanged(modelIndex, modelIndex);
    }
}

QStringList TaskModel::getTasksForGroup(const QString& groupId) const
{
    QStringList result;
    for (const auto& task : m_tasks)
    {
        if (task.groupId == groupId) result.append(task.id);
    }
    return result;
}

QVariantMap TaskModel::getTask(const QString& taskId) const
{
    int index = findTaskIndex(taskId);
    if (index >= 0)
    {
        const Task& task = m_tasks[index];
        QVariantMap map;
        map["id"] = task.id;
        map["title"] = task.title;
        map["responsible"] = task.responsible;
        map["startDate"] = task.startDate;
        map["endDate"] = task.endDate;
        map["forecastStart"] = task.forecastStart;
        map["forecastEnd"] = task.forecastEnd;
        map["status"] = static_cast<int>(task.status);
        map["groupId"] = task.groupId;
        map["comment"] = task.comment;
        map["progressCurrent"] = task.progressCurrent;
        map["progressTotal"] = task.progressTotal;
        map["localGraphState"] = static_cast<int>(task.localGraphState);
        QVariantList history;
        for (const auto& pair : task.dateHistory)
        {
            QVariantMap dates;
            dates["start"] = pair.first.toString("dd.MM.yyyy");
            dates["end"] = pair.second.toString("dd.MM.yyyy");
            history.append(dates);
        }
        map["dateHistory"] = history;
        return map;
    }
    return QVariantMap();
}

void TaskModel::moveTask(const QString& taskId, int newPosition)
{
    int oldIndex = findTaskIndex(taskId);
    if (oldIndex < 0 || oldIndex == newPosition || newPosition < 0 || newPosition >= m_tasks.size()) return;

    beginMoveRows(QModelIndex(), oldIndex, oldIndex, QModelIndex(), newPosition > oldIndex ? newPosition + 1 : newPosition);
    m_tasks.move(oldIndex, newPosition);
    endMoveRows();
}

void TaskModel::moveTaskToGroup(const QString& taskId, const QString& newGroupId, int newPosition)
{
    int index = findTaskIndex(taskId);
    if (index >= 0)
    {
        m_tasks[index].groupId = newGroupId;
        moveTask(taskId, newPosition);
        QModelIndex modelIndex = createIndex(index, 0);
        emit dataChanged(modelIndex, modelIndex);
    }
}

QList<HistoryEntry*> TaskModel::getTaskHistory(const QString& taskId) const
{
    int index = findTaskIndex(taskId);
    if (index >= 0) return m_tasks[index].history;
    return QList<HistoryEntry*>();
}

QVariantList TaskModel::getAllTasks() const
{
    QVariantList result;
    for (const auto& task : m_tasks)
    {
        QVariantMap map;
        map["id"] = task.id;
        map["title"] = task.title;
        map["responsible"] = task.responsible;
        map["startDate"] = task.startDate;
        map["endDate"] = task.endDate;
        map["forecastStart"] = task.forecastStart;
        map["forecastEnd"] = task.forecastEnd;
        map["status"] = static_cast<int>(task.status);
        map["groupId"] = task.groupId;
        map["comment"] = task.comment;
        map["progressCurrent"] = task.progressCurrent;
        map["progressTotal"] = task.progressTotal;
        map["localGraphState"] = static_cast<int>(task.localGraphState);
        QVariantList history;
        for (const auto& pair : task.dateHistory)
        {
            QVariantMap dates;
            dates["start"] = pair.first.toString("dd.MM.yyyy");
            dates["end"] = pair.second.toString("dd.MM.yyyy");
            history.append(dates);
        }
        map["dateHistory"] = history;
        result.append(map);
    }
    return result;
}

void TaskModel::clear()
{
    beginResetModel();
    for (auto& task : m_tasks)
    {
        for (auto* entry : task.history) delete entry;
    }
    m_tasks.clear();
    endResetModel();
    emit countChanged();
}
