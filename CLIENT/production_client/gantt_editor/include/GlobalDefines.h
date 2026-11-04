#ifndef GLOBALDEFINES_H
#define GLOBALDEFINES_H

#include <QString>
#include <QColor>
#include <QDate>

namespace GanttDefines {

enum class TaskStatus {
    Planned = 0,
    Completed,
    HasRisks,
    Blocked
};

enum class MilestoneStatus {
    Planned = 0,
    Completed,
    Rescheduled
};

enum class DependencyType {
    FinishToStart = 0
};

enum class ZoomLevel {
    Daily = 0,
    Weekly
};

enum class ViewMode {
    Target = 0,
    Forecast,
    Combined
};

enum class LocalGraphState {
    NotRequired = 0,
    Required,
    Attached,
    Missing
};

enum class GraphKind {
    Master = 0,
    Local
};

enum ModelRoles {
    IdRole = Qt::UserRole + 1,
    TitleRole,
    ResponsibleRole,
    StartDateRole,
    EndDateRole,
    StatusRole,
    GroupIdRole,
    ColorRole,
    DurationRole,
    HistoryRole,
    CommentRole,
    ForecastStartRole,
    ForecastEndRole,
    ProgressCurrentRole,
    ProgressTotalRole,
    LocalGraphStateRole
};

inline QColor getTaskStatusColor(TaskStatus status) {
    switch(status) {
        case TaskStatus::Planned:   return QColor(255, 255, 0);
        case TaskStatus::Completed: return QColor(0, 255, 0);
        case TaskStatus::HasRisks:  return QColor(255, 165, 0);
        case TaskStatus::Blocked:   return QColor(255, 0, 0);
        default: return QColor(200, 200, 200);
    }
}

inline QColor getMilestoneStatusColor(MilestoneStatus status) {
    switch(status) {
        case MilestoneStatus::Planned:    return QColor(255, 255, 0);
        case MilestoneStatus::Completed:  return QColor(0, 255, 0);
        case MilestoneStatus::Rescheduled: return QColor(128, 128, 128);
        default: return QColor(200, 200, 200);
    }
}

inline QString taskStatusToString(TaskStatus status) {
    switch(status) {
        case TaskStatus::Planned:   return "Запланировано";
        case TaskStatus::Completed: return "Выполнено";
        case TaskStatus::HasRisks:  return "Имеются риски";
        case TaskStatus::Blocked:   return "Блокировано";
        default: return "Неизвестно";
    }
}

inline TaskStatus stringToTaskStatus(const QString& str) {
    if (str == "Запланировано") return TaskStatus::Planned;
    if (str == "Выполнено") return TaskStatus::Completed;
    if (str == "Имеются риски") return TaskStatus::HasRisks;
    if (str == "Блокировано") return TaskStatus::Blocked;
    return TaskStatus::Planned;
}

inline QString localGraphStateToString(LocalGraphState state) {
    switch (state) {
        case LocalGraphState::NotRequired: return "notRequired";
        case LocalGraphState::Required:    return "required";
        case LocalGraphState::Attached:    return "attached";
        case LocalGraphState::Missing:     return "missing";
    }
    return "notRequired";
}

inline LocalGraphState stringToLocalGraphState(const QString& str) {
    if (str == "required") return LocalGraphState::Required;
    if (str == "attached") return LocalGraphState::Attached;
    if (str == "missing")  return LocalGraphState::Missing;
    return LocalGraphState::NotRequired;
}

inline QString graphKindToString(GraphKind kind) {
    switch (kind) {
        case GraphKind::Master: return "master";
        case GraphKind::Local:  return "local";
    }
    return "master";
}

inline GraphKind stringToGraphKind(const QString& str) {
    if (str == "local") return GraphKind::Local;
    return GraphKind::Master;
}

} // namespace GanttDefines

#endif
