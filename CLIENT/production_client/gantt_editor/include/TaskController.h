#ifndef TASKCONTROLLER_H
#define TASKCONTROLLER_H

#include <QObject>
#include "ProjectData.h"
#include "SettingsManager.h"

class TaskController : public QObject
{
    Q_OBJECT
public:
    explicit TaskController(ProjectData* projectData, SettingsManager* settingsManager, QObject *parent = nullptr);

    Q_INVOKABLE void addTask(const QString& groupId, const QString& title, const QString& responsible, const QDate& startDate, const QDate& endDate);
    Q_INVOKABLE void addTaskWithoutDates(const QString& groupId, const QString& title, const QString& responsible);
    Q_INVOKABLE void removeTask(const QString& taskId);
    Q_INVOKABLE void updateTaskDates(const QString& taskId, const QDate& newStart, const QDate& newEnd);
    Q_INVOKABLE void updateForecastDates(const QString& taskId, const QDate& newForecastStart, const QDate& newForecastEnd);
    Q_INVOKABLE void acceptForecastAsTarget(const QString& taskId);
    Q_INVOKABLE void setTaskProgress(const QString& taskId, int current, int total);
    Q_INVOKABLE void confirmRemoveTask(const QString& taskId);

signals:
    void errorOccurred(const QString& message);
    void confirmRemoveTaskWithLocalGraph(const QString& taskId);
    void taskDatesChanged(const QString& taskId);

private:
    ProjectData* m_projectData;
    SettingsManager* m_settingsManager;
    QSet<QString> m_updatingTasks;

    void updateDependentTasks(const QString& taskId, const QDate& newEndDate);
};

#endif
