#ifndef PROJECTCONTROLLER_H
#define PROJECTCONTROLLER_H

#include <QObject>
#include <QSet>
#include "ProjectData.h"
#include "ResourceManager.h"
#include "SettingsManager.h"
#include "ExportManager.h"
#include "TaskController.h"
#include "LocalGraphController.h"
#include "DependencyController.h"
#include "GanttApiClient.h"
#include "WebSocketClient.h"

class ProjectController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ProjectData* projectData READ get_projectData NOTIFY projectDataChanged)
    Q_PROPERTY(ResourceManager* resourceManager READ get_resourceManager CONSTANT)
    Q_PROPERTY(SettingsManager* settingsManager READ get_settingsManager CONSTANT)
    Q_PROPERTY(ExportManager* exportManager READ get_exportManager CONSTANT)
    Q_PROPERTY(TaskController* taskController READ get_taskController CONSTANT)
    Q_PROPERTY(LocalGraphController* localGraphController READ get_localGraphController CONSTANT)
    Q_PROPERTY(DependencyController* dependencyController READ get_dependencyController CONSTANT)
    Q_PROPERTY(WebSocketClient* webSocket READ get_webSocket CONSTANT)
    Q_PROPERTY(bool inEditMode READ get_inEditMode WRITE set_InEditMode NOTIFY inEditModeChanged)

public:
    explicit ProjectController(QObject *parent = nullptr);

    ProjectData* get_projectData() const;
    ResourceManager* get_resourceManager() const;
    SettingsManager* get_settingsManager() const;
    ExportManager* get_exportManager() const;
    TaskController* get_taskController() const;
    LocalGraphController* get_localGraphController() const;
    DependencyController* get_dependencyController() const;
    WebSocketClient* get_webSocket() const;

    bool get_inEditMode() const;
    void set_InEditMode(bool editMode);

    Q_INVOKABLE void loadFromServer(const QString& mgId, const QString& token);
    Q_INVOKABLE void exportToPng(const QString& filePath);
    Q_INVOKABLE void exportToPdf(const QString& filePath);

    Q_INVOKABLE void notifyEditingLocked();
    Q_INVOKABLE void setCursorShape(int shape);
    Q_INVOKABLE void notifyTaskCompleted();
    Q_INVOKABLE void notifyMilestoneCompleted();

signals:
    void inEditModeChanged();
    void projectLoaded();
    void projectDataChanged();
    void errorOccurred(const QString& message);
    void confirmRemoveTaskWithLocalGraph(const QString& taskId);

private slots:
    void onMastergraphLoaded(const QVariantMap& data);
    void onHolidaysLoaded(const QVariantList& holidays);
    void onAccessLoaded(bool canEdit, bool canView, const QString& reason);
    void onApiError(const QString& message);
    void onWsSnapshot(const QJsonObject& snapshot);
    void onWsDiff(const QJsonObject& diff);

private:
    ProjectData* m_projectData;
    ResourceManager* m_resourceManager;
    SettingsManager* m_settingsManager;
    ExportManager* m_exportManager;

    TaskController* m_taskController;
    LocalGraphController* m_localGraphController;
    DependencyController* m_dependencyController;

    GanttApiClient* m_apiClient;
    WebSocketClient* m_webSocket;

    bool m_inEditMode;
};

#endif
