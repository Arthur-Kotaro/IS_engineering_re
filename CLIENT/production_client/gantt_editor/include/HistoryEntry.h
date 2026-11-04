#ifndef HISTORYENTRY_H
#define HISTORYENTRY_H

#include <QObject>
#include <QDate>
#include <QDateTime>

class HistoryEntry : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString taskId READ taskId WRITE setTaskId NOTIFY taskIdChanged)
    Q_PROPERTY(QDate oldStartDate READ oldStartDate WRITE setOldStartDate NOTIFY oldStartDateChanged)
    Q_PROPERTY(QDate oldEndDate READ oldEndDate WRITE setOldEndDate NOTIFY oldEndDateChanged)
    Q_PROPERTY(QDate newStartDate READ newStartDate WRITE setNewStartDate NOTIFY newStartDateChanged)
    Q_PROPERTY(QDate newEndDate READ newEndDate WRITE setNewEndDate NOTIFY newEndDateChanged)
    Q_PROPERTY(QDateTime timestamp READ timestamp WRITE setTimestamp NOTIFY timestampChanged)

public:
    explicit HistoryEntry(QObject *parent = nullptr);
    HistoryEntry(const QString& taskId, const QDate& oldStart, const QDate& oldEnd,
                 const QDate& newStart, const QDate& newEnd, QObject *parent = nullptr);

    QString taskId() const;
    void setTaskId(const QString& id);

    QDate oldStartDate() const;
    void setOldStartDate(const QDate& date);

    QDate oldEndDate() const;
    void setOldEndDate(const QDate& date);

    QDate newStartDate() const;
    void setNewStartDate(const QDate& date);

    QDate newEndDate() const;
    void setNewEndDate(const QDate& date);

    QDateTime timestamp() const;
    void setTimestamp(const QDateTime& time);

    Q_INVOKABLE QVariantMap toMap() const;
    Q_INVOKABLE void fromMap(const QVariantMap& map);

signals:
    void taskIdChanged();
    void oldStartDateChanged();
    void oldEndDateChanged();
    void newStartDateChanged();
    void newEndDateChanged();
    void timestampChanged();

private:
    QString m_taskId;
    QDate m_oldStartDate;
    QDate m_oldEndDate;
    QDate m_newStartDate;
    QDate m_newEndDate;
    QDateTime m_timestamp;
};

#endif
