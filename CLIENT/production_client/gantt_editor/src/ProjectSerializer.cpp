#include "ProjectSerializer.h"
#include "ProjectData.h"
#include "ProjectValidator.h"
#include <QDebug>
#include <QDate>

ProjectSerializer::ProjectSerializer(QObject *parent) : QObject(parent) {}

QVariantMap ProjectSerializer::toJson(const ProjectData* project) const
{
    QVariantMap result;
    result["projectName"] = project->get_projectName();
    result["projectType"] = project->get_projectType();
    result["startDate"] = project->get_startDate().toString("dd.MM.yyyy");
    result["creationDateTime"] = project->get_creationDateTime().toString("dd.MM.yyyy hh:mm:ss");
    result["lastModifiedDateTime"] = project->get_lastModifiedDateTime().toString("dd.MM.yyyy hh:mm:ss");

    if (project->get_graphKind() == GanttDefines::GraphKind::Local)
        result["graphKind"] = "local";

    if (project->getLinkedTargetStart().isValid())
        result["linkedTargetStart"] = project->getLinkedTargetStart().toString("dd.MM.yyyy");
    if (project->getLinkedTargetEnd().isValid())
        result["linkedTargetEnd"] = project->getLinkedTargetEnd().toString("dd.MM.yyyy");
    if (project->getLinkedPredecessorEndDate().isValid())
        result["linkedPredecessorEndDate"] = project->getLinkedPredecessorEndDate().toString("dd.MM.yyyy");
    if (!project->getLinkedMasterTaskId().isEmpty())
        result["linkedMasterTaskId"] = project->getLinkedMasterTaskId();

    QVariantList groups;
    auto groupModel = project->get_groupModel();
    for (int i = 0; i < groupModel->rowCount(); ++i) {
        QVariantMap group;
        QModelIndex idx = groupModel->index(i);
        group["id"] = idx.data(Qt::UserRole + 1).toString();
        group["name"] = idx.data(Qt::DisplayRole).toString();
        group["expanded"] = idx.data(Qt::UserRole + 2).toBool();
        groups.append(group);
    }
    result["groups"] = groups;

    QVariantList tasks;
    auto taskModel = project->get_taskModel();
    for (int i = 0; i < taskModel->rowCount(); ++i) {
        QVariantMap task = taskModel->getTask(taskModel->index(i).data(GanttDefines::IdRole).toString());

        // Явно конвертируем даты в строки, чтобы избежать невалидных QDate в JSON
        QDate startDate = task["startDate"].toDate();
        QDate endDate = task["endDate"].toDate();
        QDate forecastStart = task["forecastStart"].toDate();
        QDate forecastEnd = task["forecastEnd"].toDate();

        if (startDate.isValid()) task["startDate"] = startDate.toString("dd.MM.yyyy");
        else task.remove("startDate");

        if (endDate.isValid()) task["endDate"] = endDate.toString("dd.MM.yyyy");
        else task.remove("endDate");

        if (forecastStart.isValid()) task["forecastStart"] = forecastStart.toString("dd.MM.yyyy");
        else task.remove("forecastStart");

        if (forecastEnd.isValid()) task["forecastEnd"] = forecastEnd.toString("dd.MM.yyyy");
        else task.remove("forecastEnd");

        tasks.append(task);
    }
    result["tasks"] = tasks;

    QVariantList milestones;
    auto msModel = project->get_milestoneModel();
    for (int i = 0; i < msModel->rowCount(); ++i)
    {
        QVariantMap ms = msModel->getMilestone(msModel->index(i).data(Qt::UserRole + 1).toString());

        QDate plannedDate = ms["plannedDate"].toDate();
        QDate actualDate = ms["actualDate"].toDate();

        if (plannedDate.isValid()) ms["plannedDate"] = plannedDate.toString("dd.MM.yyyy");
        else ms.remove("plannedDate");

        if (actualDate.isValid()) ms["actualDate"] = actualDate.toString("dd.MM.yyyy");
        else ms.remove("actualDate");

        milestones.append(ms);
    }
    result["milestones"] = milestones;

    QVariantList dependencies;
    auto depModel = project->get_dependencyModel();
    for (int i = 0; i < depModel->rowCount(); ++i) {
        QModelIndex idx = depModel->index(i);
        QVariantMap dep;
        dep["id"] = idx.data(Qt::UserRole + 1).toString();
        dep["predecessorId"] = idx.data(Qt::UserRole + 2).toString();
        dep["successorId"] = idx.data(Qt::UserRole + 3).toString();
        dep["type"] = idx.data(Qt::UserRole + 4).toInt();
        dependencies.append(dep);
    }
    result["dependencies"] = dependencies;

    return result;
}

bool ProjectSerializer::fromJson(ProjectData* project, const QVariantMap& json, QStringList& validationErrors)
{
    ProjectValidator validator;
    if (!validator.validate(json))
    {
        validationErrors = validator.errors();
        return false;
    }
    validationErrors.clear();

    project->clear();
    
    // Устанавливаем базовые свойства (используя публичные сеттеры ProjectData)
    project->set_ProjectName(json["projectName"].toString());
    project->set_ProjectType(json["projectType"].toString());
    project->set_StartDate(QDate::fromString(json["startDate"].toString(), "dd.MM.yyyy"));
    project->set_CreationDateTime(QDateTime::fromString(json["creationDateTime"].toString(), "dd.MM.yyyy hh:mm:ss"));
    project->set_LastModifiedDateTime(QDateTime::fromString(json["lastModifiedDateTime"].toString(), "dd.MM.yyyy hh:mm:ss"));
    
    project->set_GraphKind(GanttDefines::stringToGraphKind(json["graphKind"].toString()));
    project->set_LinkedTargetStart(QDate::fromString(json["linkedTargetStart"].toString(), "dd.MM.yyyy"));
    project->set_LinkedTargetEnd(QDate::fromString(json["linkedTargetEnd"].toString(), "dd.MM.yyyy"));
    project->set_LinkedPredecessorEndDate(QDate::fromString(json["linkedPredecessorEndDate"].toString(), "dd.MM.yyyy"));
    project->set_LinkedMasterTaskId(json["linkedMasterTaskId"].toString());

    auto groupModel = project->get_groupModel();
    auto taskModel = project->get_taskModel();
    auto msModel = project->get_milestoneModel();
    auto depModel = project->get_dependencyModel();

    // Группы (только для МГ)
    if (project->get_graphKind() == GanttDefines::GraphKind::Master) {
        for (const auto& g : json["groups"].toList()) {
            QVariantMap groupMap = g.toMap();
            groupModel->addGroupWithId(groupMap["id"].toString(), groupMap["name"].toString());
        }
    }

    // Задачи
    for (const auto& t : json["tasks"].toList()) {
        QVariantMap taskMap = t.toMap();
        QDate startDate = QDate::fromString(taskMap["startDate"].toString(), "dd.MM.yyyy");
        QDate endDate = QDate::fromString(taskMap["endDate"].toString(), "dd.MM.yyyy");
        QDate forecastStart = QDate::fromString(taskMap["forecastStart"].toString(), "dd.MM.yyyy");
        QDate forecastEnd = QDate::fromString(taskMap["forecastEnd"].toString(), "dd.MM.yyyy");
        int status = taskMap["status"].toInt();
        int progressCurrent = taskMap.contains("progressCurrent") ? taskMap["progressCurrent"].toInt() : -1;
        int progressTotal = taskMap.contains("progressTotal") ? taskMap["progressTotal"].toInt() : -1;
        GanttDefines::LocalGraphState lgs = GanttDefines::LocalGraphState::NotRequired;
        QVariant lgsVal = taskMap["localGraphState"];
        if (lgsVal.typeId() == QMetaType::Int || lgsVal.canConvert<int>()) {
            int val = lgsVal.toInt();
            if (val >= 0 && val <= 3) lgs = static_cast<GanttDefines::LocalGraphState>(val);
        } else if (lgsVal.typeId() == QMetaType::QString) {
            lgs = GanttDefines::stringToLocalGraphState(lgsVal.toString());
        }

        QString taskId = taskMap["id"].toString();
        QString groupId = taskMap["groupId"].toString();

        // Fallback для groupId (если группы нет — создаем "Без группы")
        if (project->get_graphKind() == GanttDefines::GraphKind::Master && !groupId.isEmpty()) {
            if (groupModel->getGroupIds().indexOf(groupId) < 0) {
                QString fallbackId = "fallback-group";
                if (groupModel->getGroupIds().indexOf(fallbackId) < 0)
                    groupModel->addGroupWithId(fallbackId, "Без группы");
                groupId = fallbackId;
            }
        }

        taskModel->addTaskWithId(taskId, groupId, taskMap["title"].toString(), taskMap["responsible"].toString(),
                                 startDate, endDate, forecastStart, forecastEnd, status, progressCurrent, progressTotal, static_cast<int>(lgs));

        if (!taskMap["comment"].toString().isEmpty())
            taskModel->setTaskComment(taskId, taskMap["comment"].toString());

        for (const auto& dh : taskMap["dateHistory"].toList()) {
            QVariantMap dates = dh.toMap();
            QDate oldStart = QDate::fromString(dates["start"].toString(), "dd.MM.yyyy");
            QDate oldEnd = QDate::fromString(dates["end"].toString(), "dd.MM.yyyy");
            if (oldStart.isValid() && oldEnd.isValid())
                taskModel->addDateHistory(taskId, oldStart, oldEnd);
        }
    }

    // Вехи (только для МГ)
    if (project->get_graphKind() == GanttDefines::GraphKind::Master)
    {
        for (const auto& m : json["milestones"].toList())
        {
            QVariantMap msMap = m.toMap();
            QDate plannedDate = QDate::fromString(msMap["plannedDate"].toString(), "dd.MM.yyyy");
            if (!plannedDate.isValid())
            {
                qWarning() << "Invalid plannedDate for milestone, using current date";
                plannedDate = QDate::currentDate();
            }
            msModel->addMilestone(msMap["abbreviation"].toString(), msMap["fullName"].toString(), plannedDate);
            int lastIdx = msModel->rowCount() - 1;
            QString msId = msModel->data(msModel->index(lastIdx), Qt::UserRole + 1).toString();
            if (msMap["status"].toInt() == 1) msModel->setMilestoneCompleted(msId);
            QVariantList history = msMap["rescheduleHistory"].toList();
            if (!history.isEmpty()) msModel->setRescheduleHistory(msId, history);
        }
    }

    // Зависимости
    for (const auto& d : json["dependencies"].toList()) {
        QVariantMap depMap = d.toMap();
        QString predId = depMap["predecessorId"].toString();
        QString succId = depMap["successorId"].toString();
        QVariantMap predTask = taskModel->getTask(predId);
        QVariantMap succTask = taskModel->getTask(succId);
        if (predTask.isEmpty() || succTask.isEmpty()) continue;
        bool predHasDates = predTask["startDate"].toDate().isValid() && predTask["endDate"].toDate().isValid();
        bool succHasDates = succTask["startDate"].toDate().isValid() && succTask["endDate"].toDate().isValid();
        if (!predHasDates && succHasDates) continue; // Пропускаем некорректные
        depModel->addDependency(predId, succId);
    }

    project->recalculateEndDate();
    project->set_Modified(false);
    return true;
}
