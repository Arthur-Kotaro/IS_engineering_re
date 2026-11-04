#include "ProjectValidator.h"
#include <QDate>
#include <QSet>

ProjectValidator::ProjectValidator(QObject *parent) : QObject(parent) {}

bool ProjectValidator::validate(const QVariantMap& json)
{
    m_errors.clear();

    checkRequiredFields(json);
    checkGroups(json);
    checkTasks(json);
    checkDependencies(json);
    checkMilestones(json);

    return m_errors.isEmpty();
}

void ProjectValidator::checkRequiredFields(const QVariantMap& json)
{
    if (!json.contains("projectName") || json["projectName"].toString().isEmpty())
    {
        m_errors.append("Отсутствует или пустое поле 'projectName'.");
    }
    // ЛГ определяется либо по graphKind, либо по наличию linkedMasterTaskId
    bool isLocal = (json["graphKind"].toString() == "local")
    || json.contains("linkedMasterTaskId");

    if (!isLocal)
    {
        if (!json.contains("startDate"))
        {
            m_errors.append("Отсутствует поле 'startDate'.");
        }
        else
        {
            QDate d = QDate::fromString(json["startDate"].toString(), "dd.MM.yyyy");
            if (!d.isValid())
            {
                m_errors.append("Поле 'startDate' имеет неверный формат (ожидается dd.MM.yyyy).");
            }
        }
    }
    if (!json.contains("tasks") || !json["tasks"].canConvert<QVariantList>())
    {
        m_errors.append("Отсутствует или неверный тип поля 'tasks' (ожидается список).");
    }
}

void ProjectValidator::checkGroups(const QVariantMap& json)
{
    if (!json.contains("groups")) return;
    QVariantList groups = json["groups"].toList();
    QSet<QString> groupIds;
    for (const auto& g : groups) {
        QVariantMap group = g.toMap();
        QString id = group["id"].toString();
        if (id.isEmpty()) {
            m_errors.append("Группа без поля 'id'.");
        } else if (groupIds.contains(id)) {
            m_errors.append(QString("Дублирующийся id группы: %1.").arg(id));
        } else {
            groupIds.insert(id);
        }
    }
}

void ProjectValidator::checkTasks(const QVariantMap& json)
{
    if (!json.contains("tasks")) return;
    QVariantList tasks = json["tasks"].toList();
    QVariantList groups = json["groups"].toList();
    QSet<QString> validGroupIds;
    for (const auto& g : groups) validGroupIds.insert(g.toMap()["id"].toString());

    QSet<QString> taskIds;
    for (int i = 0; i < tasks.size(); ++i)
    {
        QVariantMap task = tasks[i].toMap();
        QString id = task["id"].toString();
        
        if (id.isEmpty())
        {
            m_errors.append(QString("Задача #%1 без поля 'id'.").arg(i));
            continue;
        }
        if (taskIds.contains(id)) {
            m_errors.append(QString("Задача #%1: дублирующийся id '%2'.").arg(i).arg(id));
        }
        taskIds.insert(id);

        // Проверяем groupId только если это Мастерграфик (есть группы)
        // ЛГ определяется либо по graphKind, либо по наличию linkedMasterTaskId
        bool isLocal = (json["graphKind"].toString() == "local")
        || json.contains("linkedMasterTaskId");
        if (!isLocal && !groups.isEmpty())
        {
            QString gid = task["groupId"].toString();
            if (gid.isEmpty() || !validGroupIds.contains(gid))
            {
                m_errors.append(QString("Задача '%1': неверный или отсутствующий 'groupId' (%2).").arg(id).arg(gid));
            }
        }

        int status = task["status"].toInt();
        if (status < 0 || status > 3) {
            m_errors.append(QString("Задача '%1': неверный 'status' (%2), ожидается 0..3.").arg(id).arg(status));
        }

        QString sStr = task["startDate"].toString();
        QString eStr = task["endDate"].toString();

        bool hasStart = !sStr.isEmpty();
        bool hasEnd = !eStr.isEmpty();

        if (hasStart != hasEnd)
        {
            m_errors.append(QString("Задача '%1': задана только одна из дат (startDate/endDate).").arg(id));
        }
        else if (hasStart && hasEnd)
        {
            QDate s = QDate::fromString(sStr, "dd.MM.yyyy");
            QDate e = QDate::fromString(eStr, "dd.MM.yyyy");
            if (!s.isValid()) m_errors.append(QString("Задача '%1': неверный формат 'startDate'.").arg(id));
            if (!e.isValid()) m_errors.append(QString("Задача '%1': неверный формат 'endDate'.").arg(id));
            if (s.isValid() && e.isValid() && s > e)
            {
                m_errors.append(QString("Задача '%1': 'startDate' позже 'endDate'.").arg(id));
            }
        }
    }
}

void ProjectValidator::checkDependencies(const QVariantMap& json)
{
    if (!json.contains("dependencies")) return;
    QVariantList deps = json["dependencies"].toList();
    QVariantList tasks = json["tasks"].toList();
    QSet<QString> taskIds;
    for (const auto& t : tasks) taskIds.insert(t.toMap()["id"].toString());

    for (int i = 0; i < deps.size(); ++i) {
        QVariantMap dep = deps[i].toMap();
        QString pred = dep["predecessorId"].toString();
        QString succ = dep["successorId"].toString();
        
        if (!taskIds.contains(pred)) {
            m_errors.append(QString("Зависимость #%1: предшественник '%2' не найден.").arg(i).arg(pred));
        }
        if (!taskIds.contains(succ)) {
            m_errors.append(QString("Зависимость #%1: последователь '%2' не найден.").arg(i).arg(succ));
        }
        if (pred == succ && !pred.isEmpty()) {
            m_errors.append(QString("Зависимость #%1: задача '%2' зависит от самой себя.").arg(i).arg(pred));
        }
    }
}

void ProjectValidator::checkMilestones(const QVariantMap& json)
{
    if (!json.contains("milestones")) return;
    QVariantList ms = json["milestones"].toList();
    QSet<QString> msIds;
    for (int i = 0; i < ms.size(); ++i) {
        QVariantMap m = ms[i].toMap();
        QString id = m["id"].toString();
        if (id.isEmpty()) {
            m_errors.append(QString("Веха #%1 без поля 'id'.").arg(i));
        } else if (msIds.contains(id)) {
            m_errors.append(QString("Веха #%1: дублирующийся id '%2'.").arg(i).arg(id));
        } else {
            msIds.insert(id);
        }
        if (m["plannedDate"].toString().isEmpty()) {
            m_errors.append(QString("Веха '%1': отсутствует 'plannedDate'.").arg(id));
        }
    }
}
