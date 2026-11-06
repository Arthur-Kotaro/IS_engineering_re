#ifndef LOCALGRAPHCONTROLLER_H
#define LOCALGRAPHCONTROLLER_H

#include <QObject>
#include "ProjectData.h"
#include "ResourceManager.h"
#include "SettingsManager.h"
#include "GanttApiClient.h"

class LocalGraphController : public QObject
{
    Q_OBJECT
public:
    explicit LocalGraphController(ProjectData* projectData,
                                   ResourceManager* resourceManager,
                                   SettingsManager* settingsManager,
                                   GanttApiClient* apiClient,
                                   QObject *parent = nullptr);

    // Установка контекста ЛГ (вызывается из main.cpp)
    void setLocalGraphContext(const QString& lgId, const QString& busUrl);

    Q_INVOKABLE void markTaskRequiresLocalGraph(const QString& taskId);
    Q_INVOKABLE bool createLocalGraph(const QString& taskId);
    Q_INVOKABLE void detachLocalGraph(const QString& taskId, bool deleteFile);
    Q_INVOKABLE void refreshLocalGraphForecast(const QString& taskId);

    // CRUD для задач внутри ЛГ
    Q_INVOKABLE void addLocalTask(const QString& title, const QString& responsible, const QDate& startDate, const QDate& endDate);
    Q_INVOKABLE void updateLocalTaskDates(const QString& taskId, const QDate& newStart, const QDate& newEnd);
    Q_INVOKABLE void setLocalTaskStatus(const QString& taskId, int status);
    Q_INVOKABLE void removeLocalTask(const QString& taskId);

signals:
    void errorOccurred(const QString& message);
    void localGraphCreated(const QString& taskId);
    void localGraphRefreshed(const QString& taskId);
    void localTasksReloaded();

private slots:
    void onLocalGraphCreated(const QVariantMap& result);
    void onForecastRecalculated(const QVariantMap& result);

private:
    ProjectData* m_projectData;
    ResourceManager* m_resourceManager;
    SettingsManager* m_settingsManager;
    GanttApiClient* m_apiClient;

    QString m_lgId;
    QString m_busUrl;
    QString m_pendingTaskId;

    QString getBusUrlForTask(const QString& taskId) const;
    
    // Вспомогательный метод для отправки изменений в MG
    void pushForecastToMg(const QString& taskId, const QDate& forecastStart, const QDate& forecastEnd, int progressCurrent, int progressTotal, int status);
};

#endif
