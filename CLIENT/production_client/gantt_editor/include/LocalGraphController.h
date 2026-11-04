#ifndef LOCALGRAPHCONTROLLER_H
#define LOCALGRAPHCONTROLLER_H

#include <QObject>
#include "ProjectData.h"
#include "ResourceManager.h"
#include "SettingsManager.h"

class LocalGraphController : public QObject
{
    Q_OBJECT
public:
    explicit LocalGraphController(ProjectData* projectData, ResourceManager* resourceManager, SettingsManager* settingsManager, QObject *parent = nullptr);

    Q_INVOKABLE void markTaskRequiresLocalGraph(const QString& taskId);
    Q_INVOKABLE bool createLocalGraph(const QString& taskId);
    Q_INVOKABLE bool createLocalGraphOverwrite(const QString& taskId);
    Q_INVOKABLE bool attachExistingLocalGraph(const QString& taskId);
    Q_INVOKABLE QString getLocalGraphPath(const QString& taskId) const;
    Q_INVOKABLE bool localGraphFileExists(const QString& taskId) const;
    Q_INVOKABLE void detachLocalGraph(const QString& taskId, bool deleteFile);
    Q_INVOKABLE void refreshLocalGraphForecast(const QString& taskId, const QString& localGraphPath, bool markModified);

signals:
    void errorOccurred(const QString& message);

public:
    void refreshAllLocalGraphForecasts();

private:
    ProjectData* m_projectData;
    ResourceManager* m_resourceManager;
    SettingsManager* m_settingsManager;

    QString localGraphDirectory() const;
};

#endif
