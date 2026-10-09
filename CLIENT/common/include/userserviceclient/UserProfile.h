#ifndef USERPROFILE_H
#define USERPROFILE_H

#include "UsersServiceClient_global.h"
#include <QObject>
#include <QDateTime>
#include <QJsonObject>
#include <QStringList>

namespace UsersService {

struct USERSERVICECLIENT_EXPORT UserProfile {
    // Идентификация
    int userId = 0;
    QString userName;
    QString email;

    // ФИО
    QString lastName;
    QString firstName;
    QString middleName;

    // Пол, дата рождения
    QString gender;
    QDateTime birthDate;

    // Должность
    int positionId = 0;
    QString positionCode;
    QString positionNameRu;

    // Подразделение
    QString deptId;
    QString deptCode;
    QString deptNameRu;

    // Контакты
    QString phoneWork;
    QString phoneMobile;

    // Статус
    QString status;
    bool isBlocked = false;
    bool isDeleted = false;
    QString blockedReason;
    QDateTime blockedExpiresAt;
    QDateTime deletedAt;

    // Аудит
    QDateTime createdAt;
    QDateTime updatedAt;
    QDateTime lastLoginAt;
    QDateTime passwordUpdatedAt;

    // Роли (RU-названия) и role_codes
    QStringList roles;
    QStringList roleCodes;

    bool isSuperAdmin = false;

    QJsonObject toJson() const;
    static UserProfile fromJson(const QJsonObject& json);

    QString fullName() const;
    bool isActive() const { return status == "active" && !isBlocked && !isDeleted; }
    bool hasRole(const QString& roleCode) const { return roleCodes.contains(roleCode); }
};

} // namespace UsersService

#endif // USERPROFILE_H
