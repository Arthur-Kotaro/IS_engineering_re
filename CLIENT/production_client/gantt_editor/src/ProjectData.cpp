#include "ProjectData.h"
#include "ProjectValidator.h"
#include "ProjectSerializer.h"
#include <QDebug>
#include <QDate>

ProjectData::ProjectData(QObject *parent): QObject(parent), m_modified(false)
, m_creationDateTime(QDateTime::currentDateTime())
, m_lastModifiedDateTime(QDateTime::currentDateTime())
, m_graphKind(GanttDefines::GraphKind::Master)
{
    m_serializer = new ProjectSerializer(this);
    m_taskModel = new TaskModel(this);
    m_groupModel = new GroupModel(this);
    m_milestoneModel = new MilestoneModel(this);
    m_dependencyModel = new DependencyModel(this);
}

QString ProjectData::get_projectName() const { return m_projectName; }
void ProjectData::set_ProjectName(const QString& name)
{
    if (m_projectName != name)
    {
        m_projectName = name;
        emit projectNameChanged();
        set_Modified(true);
    }
}

QString ProjectData::get_projectType() const { return m_projectType; }
void ProjectData::set_ProjectType(const QString& type)
{
    if (m_projectType != type)
    {
        m_projectType = type;
        emit projectTypeChanged();
        set_Modified(true);
    }
}

QDate ProjectData::get_startDate() const { return m_startDate; }
void ProjectData::set_StartDate(const QDate& date)
{
    if (m_startDate != date)
    {
        m_startDate = date;
        emit startDateChanged();
        recalculateEndDate();
        set_Modified(true);
    }
}

QDate ProjectData::get_endDate() const { return m_endDate; }

void ProjectData::recalculateEndDate()
{
    QDate maxEnd = m_startDate;
    for (int i = 0; i < m_taskModel->rowCount(); ++i)
    {
        QModelIndex idx = m_taskModel->index(i);
        QDate taskEnd = idx.data(GanttDefines::EndDateRole).toDate();
        QDate forecastEnd = idx.data(GanttDefines::ForecastEndRole).toDate();
        if (taskEnd.isValid() && taskEnd > maxEnd) maxEnd = taskEnd;
        if (forecastEnd.isValid() && forecastEnd > maxEnd) maxEnd = forecastEnd;
    }
    if (m_endDate != maxEnd)
    {
        m_endDate = maxEnd;
        emit endDateChanged();
    }
}

QString ProjectData::get_filePath() const { return m_filePath; }
void ProjectData::set_FilePath(const QString& path)
{
    if (m_filePath != path)
    {
        m_filePath = path;
        emit filePathChanged();
    }
}

bool ProjectData::get_modified() const { return m_modified; }
void ProjectData::set_Modified(bool mod)
{
    if (m_modified != mod)
    {
        m_modified = mod;
        emit modifiedChanged();
    }
}

QDateTime ProjectData::get_creationDateTime() const { return m_creationDateTime; }
void ProjectData::set_CreationDateTime(const QDateTime& dt)
{
    if (m_creationDateTime != dt)
    {
        m_creationDateTime = dt;
        emit creationDateTimeChanged();
    }
}

QDateTime ProjectData::get_lastModifiedDateTime() const { return m_lastModifiedDateTime; }
void ProjectData::set_LastModifiedDateTime(const QDateTime& dt)
{
    if (m_lastModifiedDateTime != dt)
    {
        m_lastModifiedDateTime = dt;
        emit lastModifiedDateTimeChanged();
    }
}

GanttDefines::GraphKind ProjectData::get_graphKind() const { return m_graphKind; }
void ProjectData::set_GraphKind(GanttDefines::GraphKind kind)
{
    if (m_graphKind != kind)
    {
        m_graphKind = kind;
        emit graphKindChanged();
        set_Modified(true);
    }
}

int ProjectData::get_graphKindInt() const
{
    return static_cast<int>(m_graphKind);
}

void ProjectData::set_GraphKindInt(int kind)
{
    if (kind == 0 || kind == 1)
        set_GraphKind(static_cast<GanttDefines::GraphKind>(kind));
}

QDate ProjectData::getLinkedTargetStart() const { return m_linkedTargetStart; }
QDate ProjectData::getLinkedTargetEnd() const { return m_linkedTargetEnd; }
void ProjectData::set_LinkedTargetStart(const QDate& date) { m_linkedTargetStart = date; }
void ProjectData::set_LinkedTargetEnd(const QDate& date) { m_linkedTargetEnd = date; }

QDate ProjectData::getLinkedPredecessorEndDate() const { return m_linkedPredecessorEndDate; }

void ProjectData::set_LinkedPredecessorEndDate(const QDate& date)
{
    if (m_linkedPredecessorEndDate != date)
    {
        m_linkedPredecessorEndDate = date;
        emit linkedPredecessorEndDateChanged();
    }
}

QString ProjectData::getLinkedMasterTaskId() const { return m_linkedMasterTaskId; }
void ProjectData::set_LinkedMasterTaskId(const QString& taskId) { m_linkedMasterTaskId = taskId; }

void ProjectData::updateLastModified()
{
    set_LastModifiedDateTime(QDateTime::currentDateTime());
    set_Modified(false);
}

void ProjectData::refreshAll()
{
    emit m_taskModel->countChanged();
    emit m_groupModel->countChanged();
    emit m_milestoneModel->milestonesChanged();
    if (m_taskModel->rowCount() > 0)
        emit m_taskModel->dataChanged(m_taskModel->index(0), m_taskModel->index(m_taskModel->rowCount() - 1));
    if (m_groupModel->rowCount() > 0)
        emit m_groupModel->dataChanged(m_groupModel->index(0), m_groupModel->index(m_groupModel->rowCount() - 1));
}

TaskModel* ProjectData::get_taskModel() const { return m_taskModel; }
GroupModel* ProjectData::get_groupModel() const { return m_groupModel; }
MilestoneModel* ProjectData::get_milestoneModel() const { return m_milestoneModel; }
DependencyModel* ProjectData::get_dependencyModel() const { return m_dependencyModel; }

void ProjectData::clear()
{
    m_taskModel->clear();
    m_groupModel->clear();
    m_milestoneModel->clear();
    m_dependencyModel->clear();
    m_projectName.clear();
    m_projectType.clear();
    m_startDate = QDate();
    m_endDate = QDate();
    m_filePath.clear();

    m_creationDateTime = QDateTime();
    m_lastModifiedDateTime = QDateTime();

    m_graphKind = GanttDefines::GraphKind::Master;
    m_linkedTargetStart = QDate();
    m_linkedTargetEnd = QDate();
    m_linkedMasterTaskId.clear();
    m_linkedPredecessorEndDate = QDate();

    set_Modified(false);
    emit dataCleared();
}

QVariantMap ProjectData::toJson() const
{
    return m_serializer->toJson(this);
}

bool ProjectData::fromJson(const QVariantMap& json)
{
    clear();

    set_ProjectName(json.value("name").toString());
    set_ProjectType("");

    QVariantList tasks = json.value("tasks").toList();
    QDate minStart;
    for (const QVariant& tv : tasks) {
        QVariantMap t = tv.toMap();
        QString taskId = t.value("task_id").toString();
        QString groupId = t.value("group_id").toString();
        QString title = t.value("title").toString();
        int responsibleId = t.value("responsible_id").toInt();

        QString responsible = t.value("responsible_name").toString();
        if (responsible.isEmpty())
            responsible = QString::number(responsibleId);

        QDate startDate = QDate::fromString(t.value("start_date").toString(), Qt::ISODate);
        QDate endDate = QDate::fromString(t.value("end_date").toString(), Qt::ISODate);
        QDate forecastStart = QDate::fromString(t.value("forecast_start").toString(), Qt::ISODate);
        QDate forecastEnd = QDate::fromString(t.value("forecast_end").toString(), Qt::ISODate);
        int status = t.value("status").toInt();
        int progressCurrent = t.value("progress_current").toInt();
        int progressTotal = t.value("progress_total").toInt();
        int localGraphState = t.value("local_graph_state").toInt(0);

        m_taskModel->addTaskWithId(taskId, groupId, title, responsible,
                                   startDate, endDate, forecastStart, forecastEnd,
                                   status, progressCurrent, progressTotal, localGraphState);

        if (startDate.isValid() && (!minStart.isValid() || startDate < minStart))
            minStart = startDate;
    }

    if (minStart.isValid()) {
        QDate leftBoundary(minStart.year(), minStart.month(), 1);
        leftBoundary = leftBoundary.addMonths(-1);
        set_StartDate(leftBoundary);
    }

    QVariantList deps = json.value("dependencies").toList();
    for (const QVariant& dv : deps) {
        QVariantMap d = dv.toMap();
        QString predId = d.value("predecessor_id").toString();
        QString succId = d.value("successor_id").toString();
        m_dependencyModel->addDependency(predId, succId);
    }

    QVariantList groups = json.value("groups").toList();
    for (const QVariant& gv : groups) {
        QVariantMap g = gv.toMap();
        QString groupId = g.value("group_id").toString();
        QString name = g.value("name").toString();
        QString busUrl = g.value("bus_url").toString();
        m_groupModel->addGroupWithIdAndBusUrl(groupId, name, busUrl);
    }

    recalculateEndDate();
    set_Modified(false);
    qDebug() << "=== fromJson OK, tasks:" << m_taskModel->rowCount()
             << "groups:" << m_groupModel->rowCount();
    return true;
}

QDate ProjectData::getEarliestDate() const
{
    QDate earliest = QDate::currentDate();

    for (int i = 0; i < m_milestoneModel->rowCount(); ++i)
    {
        QModelIndex idx = m_milestoneModel->index(i);
        QDate plannedDate = idx.data(Qt::UserRole + 3).toDate();
        if (plannedDate.isValid() && plannedDate < earliest)
            earliest = plannedDate;

        QVariantList history = idx.data(Qt::UserRole + 7).toList();
        for (const auto& h : history)
        {
            QDate d = QDate::fromString(h.toString(), "dd.MM.yyyy");
            if (d.isValid() && d < earliest)
                earliest = d;
        }
    }

    for (int i = 0; i < m_taskModel->rowCount(); ++i)
    {
        QModelIndex idx = m_taskModel->index(i);

        QDate startDate = idx.data(GanttDefines::StartDateRole).toDate();
        if (startDate.isValid() && startDate < earliest)
            earliest = startDate;

        QDate forecastStart = idx.data(GanttDefines::ForecastStartRole).toDate();
        if (forecastStart.isValid() && forecastStart < earliest)
            earliest = forecastStart;

        QString taskId = idx.data(GanttDefines::IdRole).toString();
        QVariantMap task = m_taskModel->getTask(taskId);
        QVariantList dateHistory = task["dateHistory"].toList();
        for (const auto& dh : dateHistory)
        {
            QVariantMap dates = dh.toMap();
            QDate oldStart = QDate::fromString(dates["start"].toString(), "dd.MM.yyyy");
            if (oldStart.isValid() && oldStart < earliest)
                earliest = oldStart;
        }
    }

    return earliest;
}

QDate ProjectData::getLatestDate() const
{
    QDate latest = QDate(1900, 1, 1);

    for (int i = 0; i < m_milestoneModel->rowCount(); ++i)
    {
        QModelIndex idx = m_milestoneModel->index(i);
        QDate plannedDate = idx.data(Qt::UserRole + 3).toDate();
        if (plannedDate.isValid() && plannedDate > latest)
            latest = plannedDate;

        QVariantList history = idx.data(Qt::UserRole + 7).toList();
        for (const auto& h : history)
        {
            QDate d = QDate::fromString(h.toString(), "dd.MM.yyyy");
            if (d.isValid() && d > latest)
                latest = d;
        }
    }

    for (int i = 0; i < m_taskModel->rowCount(); ++i)
    {
        QModelIndex idx = m_taskModel->index(i);

        QDate endDate = idx.data(GanttDefines::EndDateRole).toDate();
        if (endDate.isValid() && endDate > latest)
            latest = endDate;

        QDate forecastEnd = idx.data(GanttDefines::ForecastEndRole).toDate();
        if (forecastEnd.isValid() && forecastEnd > latest)
            latest = forecastEnd;

        QString taskId = idx.data(GanttDefines::IdRole).toString();
        QVariantMap task = m_taskModel->getTask(taskId);
        QVariantList dateHistory = task["dateHistory"].toList();
        for (const auto& dh : dateHistory)
        {
            QVariantMap dates = dh.toMap();
            QDate oldEnd = QDate::fromString(dates["end"].toString(), "dd.MM.yyyy");
            if (oldEnd.isValid() && oldEnd > latest)
                latest = oldEnd;
        }
    }

    return latest;
}

QVariantMap ProjectData::getTaskStatistics() const
{
    int withDates = 0;
    int withoutDates = 0;
    int unapprovedForecasts = 0;
    int total = 0;

    for (int i = 0; i < m_taskModel->rowCount(); ++i)
    {
        QModelIndex idx = m_taskModel->index(i);
        QDate startDate = idx.data(GanttDefines::StartDateRole).toDate();
        QDate endDate = idx.data(GanttDefines::EndDateRole).toDate();
        QDate forecastStart = idx.data(GanttDefines::ForecastStartRole).toDate();
        QDate forecastEnd = idx.data(GanttDefines::ForecastEndRole).toDate();

        bool hasStart = startDate.isValid();
        bool hasEnd = endDate.isValid();
        bool hasDates = hasStart && hasEnd;
        bool hasForecast = forecastStart.isValid() && forecastEnd.isValid();

        total++;

        if (hasDates) withDates++;
        if (!hasDates) withoutDates++;
        if (!hasDates && hasForecast) unapprovedForecasts++;
    }

    QVariantMap result;
    result["withDates"] = withDates;
    result["withoutDates"] = withoutDates;
    result["unapprovedForecasts"] = unapprovedForecasts;
    result["total"] = total;
    return result;
}

QVariantList ProjectData::getConflictingTaskIds() const
{
    QVariantList result;

    if (m_graphKind != GanttDefines::GraphKind::Local) return result;
    if (!m_linkedPredecessorEndDate.isValid()) return result;

    QDate minStart = m_linkedPredecessorEndDate.addDays(1);

    for (int i = 0; i < m_taskModel->rowCount(); ++i)
    {
        QModelIndex idx = m_taskModel->index(i);
        QDate startDate = idx.data(GanttDefines::StartDateRole).toDate();
        if (!startDate.isValid()) continue;
        if (startDate < minStart)
            result.append(idx.data(GanttDefines::IdRole).toString());
    }

    return result;
}

int ProjectData::getConflictCount() const
{
    return getConflictingTaskIds().size();
}

bool ProjectData::fromSerializerJson(const QVariantMap& json)
{
    QStringList errors;
    bool ok = m_serializer->fromJson(this, json, errors);
    m_validationErrors = errors;
    return ok;
}
