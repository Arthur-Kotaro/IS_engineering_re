#ifndef PROJECTDATA_H
#define PROJECTDATA_H

#include <QObject>
#include <QDate>
#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QVariantList>
#include "TaskModel.h"
#include "GroupModel.h"
#include "MilestoneModel.h"
#include "DependencyModel.h"

class ProjectSerializer;

class ProjectData : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString projectName READ get_projectName WRITE set_ProjectName NOTIFY projectNameChanged)
    Q_PROPERTY(QString projectType READ get_projectType WRITE set_ProjectType NOTIFY projectTypeChanged)
    Q_PROPERTY(QDate startDate READ get_startDate WRITE set_StartDate NOTIFY startDateChanged)
    Q_PROPERTY(QDate endDate READ get_endDate NOTIFY endDateChanged)
    Q_PROPERTY(QString filePath READ get_filePath WRITE set_FilePath NOTIFY filePathChanged)
    Q_PROPERTY(bool modified READ get_modified WRITE set_Modified NOTIFY modifiedChanged)
    Q_PROPERTY(TaskModel* taskModel READ get_taskModel CONSTANT)
    Q_PROPERTY(GroupModel* groupModel READ get_groupModel CONSTANT)
    Q_PROPERTY(MilestoneModel* milestoneModel READ get_milestoneModel CONSTANT)
    Q_PROPERTY(DependencyModel* dependencyModel READ get_dependencyModel CONSTANT)

    Q_PROPERTY(QDateTime creationDateTime READ get_creationDateTime WRITE set_CreationDateTime NOTIFY creationDateTimeChanged)
    Q_PROPERTY(QDateTime lastModifiedDateTime READ get_lastModifiedDateTime WRITE set_LastModifiedDateTime NOTIFY lastModifiedDateTimeChanged)
    Q_PROPERTY(int graphKind READ get_graphKindInt WRITE set_GraphKindInt NOTIFY graphKindChanged)

    Q_PROPERTY(QDate linkedPredecessorEndDate READ getLinkedPredecessorEndDate WRITE set_LinkedPredecessorEndDate NOTIFY linkedPredecessorEndDateChanged)

public:
    explicit ProjectData(QObject *parent = nullptr);

    QString get_projectName() const;
    void set_ProjectName(const QString& name);

    QString get_projectType() const;
    void set_ProjectType(const QString& type);

    QDate get_startDate() const;
    void set_StartDate(const QDate& date);

    QDate get_endDate() const;
    void recalculateEndDate();

    QString get_filePath() const;
    void set_FilePath(const QString& path);

    QDateTime get_creationDateTime() const;
    void set_CreationDateTime(const QDateTime& dt);

    QDateTime get_lastModifiedDateTime() const;
    void set_LastModifiedDateTime(const QDateTime& dt);

    bool get_modified() const;
    void set_Modified(bool mod);

    GanttDefines::GraphKind get_graphKind() const;
    void set_GraphKind(GanttDefines::GraphKind kind);

    int get_graphKindInt() const;
    void set_GraphKindInt(int kind);

    Q_INVOKABLE QDate getLinkedTargetStart() const;
    Q_INVOKABLE QDate getLinkedTargetEnd() const;
    void set_LinkedTargetStart(const QDate& date);
    void set_LinkedTargetEnd(const QDate& date);

    Q_INVOKABLE QDate getLinkedPredecessorEndDate() const;
    void set_LinkedPredecessorEndDate(const QDate& date);

    Q_INVOKABLE QString getLinkedMasterTaskId() const;
    void set_LinkedMasterTaskId(const QString& taskId);

    TaskModel* get_taskModel() const;
    GroupModel* get_groupModel() const;
    MilestoneModel* get_milestoneModel() const;
    DependencyModel* get_dependencyModel() const;

    Q_INVOKABLE void clear();
    Q_INVOKABLE QVariantMap toJson() const;
    Q_INVOKABLE bool fromJson(const QVariantMap& json);
    Q_INVOKABLE void updateLastModified();
    Q_INVOKABLE void refreshAll();
    Q_INVOKABLE QDate getEarliestDate() const;
    Q_INVOKABLE QDate getLatestDate() const;

    Q_INVOKABLE QVariantMap getTaskStatistics() const;

    Q_INVOKABLE int getConflictCount() const;
    Q_INVOKABLE QVariantList getConflictingTaskIds() const;

    Q_INVOKABLE QStringList getValidationErrors() const { return m_validationErrors; }
    Q_INVOKABLE bool fromSerializerJson(const QVariantMap& json);

signals:
    void projectNameChanged();
    void projectTypeChanged();
    void startDateChanged();
    void endDateChanged();
    void filePathChanged();
    void modifiedChanged();
    void dataCleared();
    void creationDateTimeChanged();
    void lastModifiedDateTimeChanged();
    void graphKindChanged();
    void linkedPredecessorEndDateChanged();

private:
    QString m_projectName;
    QString m_projectType;
    QDate m_startDate;
    QDate m_endDate;
    QString m_filePath;
    bool m_modified;
    QDateTime m_creationDateTime;
    QDateTime m_lastModifiedDateTime;
    GanttDefines::GraphKind m_graphKind;
    QDate m_linkedTargetStart;
    QDate m_linkedTargetEnd;
    QString m_linkedMasterTaskId;
    QDate m_linkedPredecessorEndDate;

    QStringList m_validationErrors;

    ProjectSerializer* m_serializer;

    TaskModel* m_taskModel;
    GroupModel* m_groupModel;
    MilestoneModel* m_milestoneModel;
    DependencyModel* m_dependencyModel;
};

#endif
