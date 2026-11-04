#ifndef TASKMODEL_H
#define TASKMODEL_H

#include <QAbstractListModel>
#include <QDate>
#include <QList>
#include "GlobalDefines.h"
#include "HistoryEntry.h"

class TaskModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    struct Task
    {
        QString id;
        QString title;
        QString responsible;
        QDate startDate;
        QDate endDate;
        QDate forecastStart;
        QDate forecastEnd;
        GanttDefines::TaskStatus status;
        QList<QPair<QDate, QDate>> dateHistory;
        QString comment;
        QString groupId;
        QList<HistoryEntry*> history;

        int progressCurrent = -1;
        int progressTotal = -1;

        GanttDefines::LocalGraphState localGraphState = GanttDefines::LocalGraphState::NotRequired;
    };

    explicit TaskModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void addTask(const QString& groupId, const QString& title, const QString& responsible,
                             const QDate& startDate, const QDate& endDate);
    Q_INVOKABLE void addTaskWithoutDates(const QString& groupId, const QString& title,
                                         const QString& responsible);
    Q_INVOKABLE void addTaskWithId(const QString& taskId, const QString& groupId, const QString& title,
                                   const QString& responsible, const QDate& startDate, const QDate& endDate,
                                   const QDate& forecastStart, const QDate& forecastEnd, int status,
                                   int progressCurrent, int progressTotal, int localGraphState);
    Q_INVOKABLE void removeTask(const QString& taskId);
    Q_INVOKABLE void updateTask(const QString& taskId, const QString& title, const QString& responsible,
                                const QDate& startDate, const QDate& endDate, int status);
    Q_INVOKABLE void updateTaskDates(const QString& taskId, const QDate& newStart, const QDate& newEnd, bool addToHistory = true);
    Q_INVOKABLE void updateForecastDates(const QString& taskId, const QDate& newForecastStart, const QDate& newForecastEnd);
    Q_INVOKABLE void setTaskStatus(const QString& taskId, GanttDefines::TaskStatus status);
    Q_INVOKABLE void setTaskComment(const QString& taskId, const QString& comment);
    Q_INVOKABLE void setTaskProgress(const QString& taskId, int current, int total);
    Q_INVOKABLE void setLocalGraphState(const QString& taskId, int state);
    void addDateHistory(const QString& taskId, const QDate& oldStart, const QDate& oldEnd);
    Q_INVOKABLE QStringList getTasksForGroup(const QString& groupId) const;
    Q_INVOKABLE QVariantMap getTask(const QString& taskId) const;
    Q_INVOKABLE void moveTask(const QString& taskId, int newPosition);
    Q_INVOKABLE void moveTaskToGroup(const QString& taskId, const QString& newGroupId, int newPosition);
    Q_INVOKABLE QList<HistoryEntry*> getTaskHistory(const QString& taskId) const;
    Q_INVOKABLE QVariantList getAllTasks() const;

    void clear();

signals:
    void countChanged();
    void taskDatesChanged(const QString& taskId);
    void taskForecastDatesChanged(const QString& taskId);
    void taskProgressChanged(const QString& taskId);
    void taskLocalGraphStateChanged(const QString& taskId);

private:
    QList<Task> m_tasks;
    QString generateId() const;
    int findTaskIndex(const QString& taskId) const;
};

#endif
