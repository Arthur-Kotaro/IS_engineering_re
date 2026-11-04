#ifndef MILESTONEMODEL_H
#define MILESTONEMODEL_H

#include <QAbstractListModel>
#include <QDate>
#include "GlobalDefines.h"

class MilestoneModel : public QAbstractListModel
{
    Q_OBJECT

public:
    struct Milestone
    {
        QString id;
        QString abbreviation;
        QString fullName;
        QString tooltip;
        int weekOffset;
        QDate plannedDate;
        QDate actualDate;
        GanttDefines::MilestoneStatus status;
        QList<QDate> rescheduleHistory;
    };

    explicit MilestoneModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void addMilestone(const QString& abbreviation, const QString& fullName, const QDate& plannedDate);
    Q_INVOKABLE void removeMilestone(const QString& milestoneId); 
    Q_INVOKABLE void setMilestoneCompleted(const QString& milestoneId);
    Q_INVOKABLE void rescheduleMilestone(const QString& milestoneId, const QDate& newDate);
    Q_INVOKABLE void setRescheduleHistory(const QString& milestoneId, const QVariantList& history);
    Q_INVOKABLE void addRescheduleHistory(const QString& milestoneId, const QDate& date);
    Q_INVOKABLE void loadFromTemplate(const QVariantList& templates, const QDate& projectStartDate);
    Q_INVOKABLE QVariantMap getMilestone(const QString& milestoneId) const;
    Q_INVOKABLE QVariantList getAllMilestones() const;
    Q_INVOKABLE QDate getFirstMilestoneDate();
    Q_INVOKABLE QDate getOriginalFirstMilestoneDate();
    Q_INVOKABLE QDate getLastMilestoneDate();

    void clear();

signals:
    void milestoneStatusChanged(const QString& milestoneId);
    void milestoneDateChanged(const QString& milestoneId);
    void milestonesChanged(); // Сигнал для обновления диапазона календаря

private:
    QList<Milestone> m_milestones;
    QDate m_originalFirstDate;
    QString generateId() const;
    int findMilestoneIndex(const QString& milestoneId) const;
    
    // Вспомогательный метод для корректировки даты на ближайший понедельник
    QDate adjustToNextMonday(QDate date);
};

#endif
