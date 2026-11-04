#include "DependencyModel.h"
#include <QUuid>

DependencyModel::DependencyModel(QObject *parent) : QAbstractListModel(parent) {}

int DependencyModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return m_dependencies.count();
}

QHash<int, QByteArray> DependencyModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[Qt::UserRole + 1] = "dependencyId";
    roles[Qt::UserRole + 2] = "predecessorId";
    roles[Qt::UserRole + 3] = "successorId";
    roles[Qt::UserRole + 4] = "type";
    return roles;
}

QVariant DependencyModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_dependencies.size())
        return QVariant();

    const Dependency &dep = m_dependencies[index.row()];
    switch (role) {
        case Qt::UserRole + 1: return dep.id;
        case Qt::UserRole + 2: return dep.predecessorId;
        case Qt::UserRole + 3: return dep.successorId;
        case Qt::UserRole + 4: return static_cast<int>(dep.type);
        default: return QVariant();
    }
}

QString DependencyModel::generateId() const {
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

int DependencyModel::findDependencyIndex(const QString& depId) const {
    for (int i = 0; i < m_dependencies.size(); ++i) {
        if (m_dependencies[i].id == depId)
            return i;
    }
    return -1;
}

bool DependencyModel::hasCycleRecursive(const QString& current, const QString& target, QSet<QString>& visited) const {
    if (current == target) return true;
    if (visited.contains(current)) return false;
    visited.insert(current);
    
    for (const auto& dep : m_dependencies) {
        if (dep.predecessorId == current) {
            if (hasCycleRecursive(dep.successorId, target, visited))
                return true;
        }
    }
    return false;
}

bool DependencyModel::hasCycle(const QString& startTaskId, const QString& newPredecessorId) const {
    QSet<QString> visited;
    return hasCycleRecursive(newPredecessorId, startTaskId, visited);
}

bool DependencyModel::addDependency(const QString& predecessorId, const QString& successorId) {
    if (hasCycle(successorId, predecessorId)) {
        qDebug() << "addDependency: cycle detected!";
        return false;
    }
    
    for (const auto& dep : m_dependencies) {
        if (dep.predecessorId == predecessorId && dep.successorId == successorId) {
            return false;
        }
    }
    
    beginInsertRows(QModelIndex(), m_dependencies.size(), m_dependencies.size());
    Dependency dep;
    dep.id = generateId();
    dep.predecessorId = predecessorId;
    dep.successorId = successorId;
    dep.type = GanttDefines::DependencyType::FinishToStart;
    m_dependencies.append(dep);
    qDebug() << "Dependency added:" << predecessorId << "->" << successorId;
    endInsertRows();
    
    emit dependencyAdded(predecessorId, successorId);
    return true;
}

void DependencyModel::removeDependency(const QString& dependencyId) {
    int index = findDependencyIndex(dependencyId);
    if (index >= 0) {
        beginRemoveRows(QModelIndex(), index, index);
        m_dependencies.removeAt(index);
        endRemoveRows();
        emit dependencyRemoved(dependencyId);
    }
}

void DependencyModel::removeDownstreamDependency(const QString& taskId) {
    for (int i = m_dependencies.size() - 1; i >= 0; --i) {
        if (m_dependencies[i].predecessorId == taskId) {
            QString depId = m_dependencies[i].id;
            beginRemoveRows(QModelIndex(), i, i);
            m_dependencies.removeAt(i);
            endRemoveRows();
            emit dependencyRemoved(depId);
            return;
        }
    }
}

void DependencyModel::removeUpstreamDependency(const QString& taskId) {
    for (int i = m_dependencies.size() - 1; i >= 0; --i) {
        if (m_dependencies[i].successorId == taskId) {
            QString depId = m_dependencies[i].id;
            beginRemoveRows(QModelIndex(), i, i);
            m_dependencies.removeAt(i);
            endRemoveRows();
            emit dependencyRemoved(depId);
            return;
        }
    }
}

void DependencyModel::removeDependenciesForTask(const QString& taskId) {
    QList<int> indicesToRemove;
    for (int i = 0; i < m_dependencies.size(); ++i) {
        if (m_dependencies[i].predecessorId == taskId || m_dependencies[i].successorId == taskId) {
            indicesToRemove.append(i);
        }
    }
    
    for (int i = indicesToRemove.size() - 1; i >= 0; --i) {
        beginRemoveRows(QModelIndex(), indicesToRemove[i], indicesToRemove[i]);
        m_dependencies.removeAt(indicesToRemove[i]);
        endRemoveRows();
    }
}

QStringList DependencyModel::getPredecessors(const QString& taskId) const {
    QStringList result;
    for (const auto& dep : m_dependencies) {
        if (dep.successorId == taskId) {
            result.append(dep.predecessorId);
        }
    }
    return result;
}

QStringList DependencyModel::getSuccessors(const QString& taskId) const {
    QStringList result;
    for (const auto& dep : m_dependencies) {
        if (dep.predecessorId == taskId) {
            result.append(dep.successorId);
        }
    }
    return result;
}

QList<QPair<QString, QString>> DependencyModel::getDependencyLines() const {
    QList<QPair<QString, QString>> result;
    for (const auto& dep : m_dependencies) {
        result.append(qMakePair(dep.predecessorId, dep.successorId));
    }
    return result;
}

void DependencyModel::clear() {
    beginResetModel();
    m_dependencies.clear();
    endResetModel();
}
