#ifndef LOCALGRAPHCONTROLLER_H
#define LOCALGRAPHCONTROLLER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QDate>
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

    void setLocalGraphContext(const QString& lgId, const QString& busUrl,
                              int projectId, int responsibleId, const QString& masterTaskId);

    Q_INVOKABLE void markTaskRequiresLocalGraph(const QString& taskId);
    Q_INVOKABLE bool createLocalGraph(const QString& taskId);
    Q_INVOKABLE void detachLocalGraph(const QString& taskId, bool deleteFile);
    Q_INVOKABLE void refreshLocalGraphForecast(const QString& taskId);

    Q_INVOKABLE void addLocalTask(const QString& title, const QString& responsible,
                                  const QDate& startDate, const QDate& endDate);
    Q_INVOKABLE void updateLocalTaskDates(const QString& taskId, const QDate& newStart, const QDate& newEnd);
    Q_INVOKABLE void setLocalTaskStatus(const QString& taskId, int status);
    Q_INVOKABLE void removeLocalTask(const QString& taskId);

    Q_INVOKABLE void addLocalDependency(const QString& predecessorId, const QString& successorId);
    Q_INVOKABLE void removeLocalDependency(const QString& depId);
    Q_INVOKABLE void loadLocalDependencies();

signals:
    void errorOccurred(const QString& message);
    void localGraphCreated(const QString& taskId);
    void localGraphRefreshed(const QString& taskId);
    void localTasksReloaded();
    void localDependenciesReloaded();
    void cascadeUpdateRequested(const QVariantList& successors, const QString& baseReason);

private slots:
    void onLocalGraphCreated(const QVariantMap& result);
    void onForecastRecalculated(const QVariantMap& result);
    void onLgDependencyCreated(const QVariantMap& result);
    void onLgDependencyDeleted(const QString& depId);
    void onLgDependenciesLoaded(const QVariantList& deps);

private:
    ProjectData* m_projectData;
    ResourceManager* m_resourceManager;
    SettingsManager* m_settingsManager;
    GanttApiClient* m_apiClient;

    QNetworkAccessManager* m_networkManager;

    QString m_lgId;
    QString m_busUrl;
    QString m_masterTaskId;
    int m_projectId = 0;
    int m_responsibleId = 0;
    QString m_pendingTaskId;

    QString getBusUrlForTask(const QString& taskId) const;
    void pushForecastToMg(const QString& taskId, const QDate& forecastStart, const QDate& forecastEnd,
                          int progressCurrent, int progressTotal, int status);
};

#endif
