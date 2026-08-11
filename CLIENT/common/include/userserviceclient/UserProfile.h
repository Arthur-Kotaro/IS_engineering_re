#ifndef USERPROFILE_H
#define USERPROFILE_H

#include "UsersServiceClient_global.h"
#include <QObject>
#include <QDateTime>
#include <QJsonObject>
#include <QStringList>

namespace UsersService {

struct USERSERVICECLIENT_EXPORT UserProfile {
    // Основная информация
    int userId = 0;
    QString userName;
    QString email;
    QString gender;
    QDateTime birthDate;
    QString deptCode;
    QString phoneWork;
    QString phoneMobile;
    
    // Статус
    QString status;           // "active", "blocked", "deleted"
    bool isBlocked = false;
    bool isDeleted = false;
    QString blockedReason;
    QDateTime blockedExpiresAt;
    QDateTime deletedAt;
    
    // Даты
    QDateTime createdAt;
    QDateTime updatedAt;
    QDateTime lastLoginAt;
    QDateTime passwordUpdatedAt;
    
    // Роли и проекты
    QStringList roles;
    QList<QJsonObject> projects;
    
    QJsonObject toJson() const;
    static UserProfile fromJson(const QJsonObject& json);
    
    // Вспомогательные методы
    bool isActive() const { return status == "active" && !isBlocked && !isDeleted; }
    bool hasRole(const QString& role) const { return roles.contains(role); }
    
    // Метод для определения каталога виджетов по роли
    QString getWidgetsPath() const;
};

} // namespace UsersService

#endif // USERPROFILE_H
