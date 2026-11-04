#ifndef DEPENDENCYMODEL_H
#define DEPENDENCYMODEL_H

#include <QAbstractListModel>
#include <QMap>
#include "GlobalDefines.h"

class DependencyModel : public QAbstractListModel
{
    Q_OBJECT

public:
    struct Dependency {
        QString id;
        QString predecessorId;
        QString successorId;
        GanttDefines::DependencyType type;
    };

    explicit DependencyModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE bool addDependency(const QString& predecessorId, const QString& successorId);
    Q_INVOKABLE void removeDependency(const QString& dependencyId);
    Q_INVOKABLE void removeDependenciesForTask(const QString& taskId);
    Q_INVOKABLE void removeDownstreamDependency(const QString& taskId);
    Q_INVOKABLE void removeUpstreamDependency(const QString& taskId);
    Q_INVOKABLE QStringList getPredecessors(const QString& taskId) const;
    Q_INVOKABLE QStringList getSuccessors(const QString& taskId) const;
    Q_INVOKABLE bool hasCycle(const QString& startTaskId, const QString& newPredecessorId) const;
    Q_INVOKABLE QList<QPair<QString, QString>> getDependencyLines() const;

    void clear();

signals:
    void dependencyAdded(const QString& predecessorId, const QString& successorId);
    void dependencyRemoved(const QString& dependencyId);

private:
    QList<Dependency> m_dependencies;
    QString generateId() const;
    int findDependencyIndex(const QString& depId) const;
    bool hasCycleRecursive(const QString& current, const QString& target, QSet<QString>& visited) const;
};

#endif
