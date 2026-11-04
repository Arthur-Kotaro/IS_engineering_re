#include "HistoryEntry.h"

HistoryEntry::HistoryEntry(QObject *parent) : QObject(parent) {}

HistoryEntry::HistoryEntry(const QString& taskId, const QDate& oldStart, const QDate& oldEnd,
                           const QDate& newStart, const QDate& newEnd, QObject *parent)
    : QObject(parent)
    , m_taskId(taskId)
    , m_oldStartDate(oldStart)
    , m_oldEndDate(oldEnd)
    , m_newStartDate(newStart)
    , m_newEndDate(newEnd)
    , m_timestamp(QDateTime::currentDateTime())
{}

QString HistoryEntry::taskId() const { return m_taskId; }
void HistoryEntry::setTaskId(const QString& id) {
    if (m_taskId != id) {
        m_taskId = id;
        emit taskIdChanged();
    }
}

QDate HistoryEntry::oldStartDate() const { return m_oldStartDate; }
void HistoryEntry::setOldStartDate(const QDate& date) {
    if (m_oldStartDate != date) {
        m_oldStartDate = date;
        emit oldStartDateChanged();
    }
}

QDate HistoryEntry::oldEndDate() const { return m_oldEndDate; }
void HistoryEntry::setOldEndDate(const QDate& date) {
    if (m_oldEndDate != date) {
        m_oldEndDate = date;
        emit oldEndDateChanged();
    }
}

QDate HistoryEntry::newStartDate() const { return m_newStartDate; }
void HistoryEntry::setNewStartDate(const QDate& date) {
    if (m_newStartDate != date) {
        m_newStartDate = date;
        emit newStartDateChanged();
    }
}

QDate HistoryEntry::newEndDate() const { return m_newEndDate; }
void HistoryEntry::setNewEndDate(const QDate& date) {
    if (m_newEndDate != date) {
        m_newEndDate = date;
        emit newEndDateChanged();
    }
}

QDateTime HistoryEntry::timestamp() const { return m_timestamp; }
void HistoryEntry::setTimestamp(const QDateTime& time) {
    if (m_timestamp != time) {
        m_timestamp = time;
        emit timestampChanged();
    }
}

QVariantMap HistoryEntry::toMap() const {
    QVariantMap map;
    map["taskId"] = m_taskId;
    map["oldStartDate"] = m_oldStartDate.toString("dd.MM.yyyy");
    map["oldEndDate"] = m_oldEndDate.toString("dd.MM.yyyy");
    map["newStartDate"] = m_newStartDate.toString("dd.MM.yyyy");
    map["newEndDate"] = m_newEndDate.toString("dd.MM.yyyy");
    map["timestamp"] = m_timestamp.toString("dd.MM.yyyy hh:mm:ss");
    return map;
}

void HistoryEntry::fromMap(const QVariantMap& map) {
    m_taskId = map["taskId"].toString();
    m_oldStartDate = QDate::fromString(map["oldStartDate"].toString(), "dd.MM.yyyy");
    m_oldEndDate = QDate::fromString(map["oldEndDate"].toString(), "dd.MM.yyyy");
    m_newStartDate = QDate::fromString(map["newStartDate"].toString(), "dd.MM.yyyy");
    m_newEndDate = QDate::fromString(map["newEndDate"].toString(), "dd.MM.yyyy");
    m_timestamp = QDateTime::fromString(map["timestamp"].toString(), "dd.MM.yyyy hh:mm:ss");
}
