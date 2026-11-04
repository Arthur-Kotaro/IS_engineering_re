#include "GroupModel.h"
#include <QUuid>

GroupModel::GroupModel(QObject *parent) : QAbstractListModel(parent) {}

int GroupModel::rowCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return m_groups.count();
}

QHash<int, QByteArray> GroupModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[Qt::DisplayRole] = "groupName";
    roles[Qt::UserRole + 1] = "groupId";
    roles[Qt::UserRole + 2] = "expanded";
    return roles;
}

QVariant GroupModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_groups.size())
        return QVariant();

    const Group &group = m_groups[index.row()];
    switch (role) {
        case Qt::DisplayRole: return group.name;
        case Qt::UserRole + 1: return group.id;
        case Qt::UserRole + 2: return group.expanded;
        default: return QVariant();
    }
}

QString GroupModel::generateId() const {
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

int GroupModel::findGroupIndex(const QString& groupId) const {
    for (int i = 0; i < m_groups.size(); ++i) {
        if (m_groups[i].id == groupId)
            return i;
    }
    return -1;
}

void GroupModel::addGroup(const QString& name) {
    beginInsertRows(QModelIndex(), m_groups.size(), m_groups.size());
    qDebug() << "Group added:" << name;
    Group group;
    group.id = generateId();
    group.name = name;
    group.expanded = true;
    m_groups.append(group);
    endInsertRows();
    emit countChanged();
}

void GroupModel::addGroupWithId(const QString& id, const QString& name) {
    beginInsertRows(QModelIndex(), m_groups.size(), m_groups.size());
    Group group;
    group.id = id;
    group.name = name;
    group.expanded = true;
    m_groups.append(group);
    endInsertRows();
    emit countChanged();
}

void GroupModel::addGroupAt(int position, const QString& name) {
    if (position < 0 || position > m_groups.size()) position = m_groups.size();
    beginInsertRows(QModelIndex(), position, position);
    Group group;
    group.id = generateId();
    group.name = name;
    group.expanded = true;
    m_groups.insert(position, group);
    endInsertRows();
    emit countChanged();
}

void GroupModel::removeGroup(const QString& groupId) {
    int index = findGroupIndex(groupId);
    if (index >= 0) {
        beginRemoveRows(QModelIndex(), index, index);
        m_groups.removeAt(index);
        endRemoveRows();
        emit countChanged();
    }
}

void GroupModel::renameGroup(const QString& groupId, const QString& newName) {
    qDebug() << "Group renamed:" << groupId << "->" << newName;
    int index = findGroupIndex(groupId);
    if (index >= 0) {
        m_groups[index].name = newName;
        QModelIndex modelIndex = createIndex(index, 0);
        emit dataChanged(modelIndex, modelIndex);
    }
}

void GroupModel::setGroupExpanded(const QString& groupId, bool expanded) {
    int index = findGroupIndex(groupId);
    if (index >= 0) {
        m_groups[index].expanded = expanded;
        emit groupExpandedChanged(groupId);
        QModelIndex modelIndex = createIndex(index, 0);
        emit dataChanged(modelIndex, modelIndex);
    }
}

void GroupModel::moveGroup(const QString& groupId, int newPosition) {
    int oldIndex = findGroupIndex(groupId);
    if (oldIndex < 0 || oldIndex == newPosition || newPosition < 0 || newPosition >= m_groups.size())
        return;
    
    beginMoveRows(QModelIndex(), oldIndex, oldIndex, QModelIndex(), newPosition > oldIndex ? newPosition + 1 : newPosition);
    m_groups.move(oldIndex, newPosition);
    endMoveRows();
}

QVariantMap GroupModel::getGroup(const QString& groupId) const {
    int index = findGroupIndex(groupId);
    if (index >= 0) {
        const Group& group = m_groups[index];
        QVariantMap map;
        map["id"] = group.id;
        map["name"] = group.name;
        map["expanded"] = group.expanded;
        return map;
    }
    return QVariantMap();
}

QStringList GroupModel::getGroupIds() const {
    QStringList ids;
    for (const auto& group : m_groups) {
        ids.append(group.id);
    }
    return ids;
}

void GroupModel::clear() {
    beginResetModel();
    m_groups.clear();
    endResetModel();
    emit countChanged();
}

QString GroupModel::getGroupId(int index) const {
    if (index < 0 || index >= m_groups.size()) return "";
    return m_groups[index].id;
}

QString GroupModel::getGroupName(int index) const {
    if (index < 0 || index >= m_groups.size()) return "";
    return m_groups[index].name;
}

bool GroupModel::isExpanded(int index) const {
    if (index < 0 || index >= m_groups.size()) return false;
    return m_groups[index].expanded;
}
