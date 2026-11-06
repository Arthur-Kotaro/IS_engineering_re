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

    Q_INVOKABLE void markTaskRequiresLocalGraph(const QString& taskId);
    Q_INVOKABLE bool createLocalGraph(const QString& taskId);
    Q_INVOKABLE bool createLocalGraphOverwrite(const QString& taskId);
    Q_INVOKABLE bool attachExistingLocalGraph(const QString& taskId);
    Q_INVOKABLE void detachLocalGraph(const QString& taskId, bool deleteFile);
    Q_INVOKABLE void refreshLocalGraphForecast(const QString& taskId);

signals:
    void errorOccurred(const QString& message);
    void localGraphCreated(const QString& taskId);
    void localGraphRefreshed(const QString& taskId);

private slots:
    void onLocalGraphCreated(const QVariantMap& result);
    void onForecastRecalculated(const QVariantMap& result);

private:
    ProjectData* m_projectData;
    ResourceManager* m_resourceManager;
    SettingsManager* m_settingsManager;
    GanttApiClient* m_apiClient;

    QString m_pendingTaskId;

    QString getBusUrlForTask(const QString& taskId) const;
};

#endif
