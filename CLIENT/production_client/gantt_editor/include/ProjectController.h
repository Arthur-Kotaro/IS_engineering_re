#ifndef PROJECTCONTROLLER_H
#define PROJECTCONTROLLER_H

#include <QObject>
#include <QSet>
#include <QJsonArray>
#include <QVariantList>
#include <functional>
#include "ProjectData.h"
#include "ResourceManager.h"
#include "SettingsManager.h"
#include "ExportManager.h"
#include "TaskController.h"
#include "LocalGraphController.h"
#include "DependencyController.h"
#include "GanttApiClient.h"
#include "WebSocketClient.h"
#include "TokenRefreshClient.h"

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
    Q_PROPERTY(TokenRefreshClient* tokenRefreshClient READ get_tokenRefreshClient CONSTANT)
    Q_PROPERTY(bool inEditMode READ get_inEditMode WRITE set_InEditMode NOTIFY inEditModeChanged)
    Q_PROPERTY(bool isPure READ get_isPure NOTIFY isPureChanged)
    Q_PROPERTY(bool canEdit READ get_canEdit NOTIFY canEditChanged)
    Q_PROPERTY(bool isApproved READ get_isApproved NOTIFY isApprovedChanged)
    Q_PROPERTY(bool isLocalGraph READ get_isLocalGraph NOTIFY graphKindChanged)
    Q_PROPERTY(QVariantList projectMembers READ get_projectMembers NOTIFY projectMembersChanged)

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
    TokenRefreshClient* get_tokenRefreshClient() const;

    bool get_inEditMode() const;
    void set_InEditMode(bool editMode);

    bool get_isPure() const;
    bool get_canEdit() const;
    bool get_isApproved() const;
    bool get_isLocalGraph() const;
    QVariantList get_projectMembers() const { return m_projectMembers; }

    Q_INVOKABLE void loadFromServer(const QString& mgId, const QString& token);
    Q_INVOKABLE bool openLocalGraphProcess(const QString& taskId, const QString& busUrl);
    Q_INVOKABLE void openForProject(int projectId, bool isReadOnly, const QString& token);

    void initLocalGraphData(const QVariantMap& lgData, const QVariantList& tasks, const QString& busUrl);

    // Группы МГ
    Q_INVOKABLE void loadDefaultGroups();
    Q_INVOKABLE void createGroups(const QStringList& groupIds);
    Q_INVOKABLE void createCustomGroup(const QString& name);
    Q_INVOKABLE void deleteGroup(const QString& groupId);

    // Задачи: создание
    Q_INVOKABLE void createTask(const QString& groupId, const QString& title,
                                 const QString& responsible, const QDate& startDate, const QDate& endDate);
    Q_INVOKABLE void createTaskWithoutDates(const QString& groupId, const QString& title,
                                             const QString& responsible);
    Q_INVOKABLE void createTasksBatch(const QVariantList& tasks, const QVariantList& dependencies);

    // Задачи: обновление (МГ)
    Q_INVOKABLE void updateTaskDates(const QString& taskId, const QDate& start, const QDate& end,
                                     const QString& reason = QString());
    Q_INVOKABLE void updateTaskForecast(const QString& taskId, const QDate& fs, const QDate& fe,
                                        const QString& reason = QString());
    Q_INVOKABLE void updateTaskResponsible(const QString& taskId, int responsibleId);
    Q_INVOKABLE void updateTaskStatus(const QString& taskId, int status);
    Q_INVOKABLE void updateTaskProgress(const QString& taskId, int current, int total);
    Q_INVOKABLE void updateTaskComment(const QString& taskId, const QString& comment);
    Q_INVOKABLE void updateTaskTitle(const QString& taskId, const QString& title);
    Q_INVOKABLE void deleteTask(const QString& taskId);

    // Задачи: обновление (ЛГ)
    Q_INVOKABLE void updateLocalTaskDates(const QString& taskId, const QDate& start, const QDate& end);

    // Вехи
    Q_INVOKABLE void updateMilestoneDate(const QString& milestoneId, const QDate& newDate,
                                         const QString& reason = QString());
    Q_INVOKABLE void completeMilestone(const QString& milestoneId, const QDate& actualDate);

    // Статус МГ
    Q_INVOKABLE void approveMastergraph(const QString& reason = QString());

    // Зависимости
    Q_INVOKABLE void createDependency(const QString& predId, const QString& succId);
    Q_INVOKABLE void deleteDependency(const QString& depId);

    // История
    Q_INVOKABLE void loadTaskHistory(const QString& taskId);

    // Участники проекта
    Q_INVOKABLE void loadProjectMembers();

    // Перезагрузка
    Q_INVOKABLE void reloadMastergraph();

    Q_INVOKABLE void exportToPng(const QString& filePath);
    Q_INVOKABLE void exportToPdf(const QString& filePath);

    Q_INVOKABLE void notifyEditingLocked();
    Q_INVOKABLE void setCursorShape(int shape);
    Q_INVOKABLE void notifyTaskCompleted();
    Q_INVOKABLE void notifyMilestoneCompleted();

    Q_INVOKABLE void saveProject();

    Q_INVOKABLE void renameGroup(const QString& groupId, const QString& newName);
    Q_INVOKABLE void createGroupsWithPosition(const QStringList& groupIds, const QString& position, const QString& afterGroupId = QString());
    Q_INVOKABLE void createCustomGroupWithPosition(const QString& name, const QString& position, const QString& afterGroupId = QString());

signals:
    void inEditModeChanged();
    void projectLoaded();
    void projectDataChanged();
    void errorOccurred(const QString& message);
    void confirmRemoveTaskWithLocalGraph(const QString& taskId);
    void saveCompleted(bool success, const QString& message);

    void isPureChanged();
    void canEditChanged();
    void isApprovedChanged();
    void graphKindChanged();
    void wsConnectionStateChanged(const QString& state);

    void defaultGroupsLoaded(const QVariantList& groups);
    void groupCreated(bool success, const QString& message);
    void groupDeleted(bool success, const QString& message);

    void taskCreated(bool success, const QString& message);
    void taskUpdated(bool success, const QString& message);
    void taskDeleted(bool success, const QString& message);
    void tasksBatchCreated(bool success, const QString& message);
    void milestoneUpdated(bool success, const QString& message);
    void statusChanged(bool success, const QString& message);
    void dependencyCreated(bool success, const QString& message);
    void dependencyDeleted(bool success, const QString& message);
    void taskHistoryLoaded(const QString& taskId, const QVariantList& events);
    void titleConflictDetected(const QVariantList& conflicts);
    void cascadeUpdateRequested(const QVariantList& successors, const QString& baseReason);
    void projectMembersChanged();

private slots:
    void onMastergraphLoaded(const QVariantMap& data);
    void onHolidaysLoaded(const QVariantList& holidays);
    void onAccessLoaded(bool canEdit, bool canView, const QString& reason);
    void onApiError(const QString& message);
    void onWsSnapshot(const QJsonObject& snapshot);
    void onWsDiff(const QJsonObject& diff);
    void onLocalGraphLoaded(const QVariantMap& lgData, const QVariantList& tasks);
    void onLocalTasksReloaded();
    void onProjectListLoaded(const QJsonArray& editable, const QJsonArray& viewable);

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
    TokenRefreshClient* m_tokenRefreshClient;

    bool m_inEditMode;
    QString m_token;
    QString m_busUrl;
    QString m_mgId;
    QString m_lgTaskId;
    int m_lgProjectId = 0;
    int m_lgResponsibleId = 0;
    int m_projectId = 0;
    int m_currentMgVersion = 0;

    QJsonArray m_editableProjects;
    QJsonArray m_viewableProjects;
    QVariantList m_projectMembers;

    void _put(const QString& path, const QJsonObject& body,
              std::function<void(int, const QByteArray&)> onDone);
    void _post(const QString& path, const QJsonObject& body,
               std::function<void(int, const QByteArray&)> onDone);
    void _del(const QString& path,
              std::function<void(int, const QByteArray&)> onDone);
    void _get(const QString& path,
              std::function<void(int, const QByteArray&)> onDone);
};

#endif // PROJECTCONTROLLER_H
