#ifndef DEPENDENCYCONTROLLER_H
#define DEPENDENCYCONTROLLER_H

#include <QObject>
#include "ProjectData.h"
#include "SettingsManager.h"

class DependencyController : public QObject
{
    Q_OBJECT
public:
    explicit DependencyController(ProjectData* projectData, SettingsManager* settingsManager, QObject *parent = nullptr);

    Q_INVOKABLE void addDependency(const QString& predecessorId, const QString& successorId);
    Q_INVOKABLE void removeDependency(const QString& dependencyId);

signals:
    void errorOccurred(const QString& message);

private:
    ProjectData* m_projectData;
    SettingsManager* m_settingsManager;
};

#endif
