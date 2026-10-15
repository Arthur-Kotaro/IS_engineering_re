#ifndef USERDATAMODELS_H
#define USERDATAMODELS_H

#include "UsersServiceClient_global.h"

#include <QDateTime>
#include <QJsonObject>
#include <QList>
#include <QString>

namespace UsersService
{

struct USERSERVICECLIENT_EXPORT ProjectRole
{
    int projectId = 0;
    QString projectName;
    QString role;
    QDateTime lastModified;

    QJsonObject toJson() const;
    static ProjectRole fromJson(const QJsonObject &json);
};

struct USERSERVICECLIENT_EXPORT UserInfo
{
    QString userId;
    QString email;
    QString fullName;
    QString position;
    QDateTime passwordExpiresAt;
    QList<ProjectRole> projectRoles;

    QJsonObject toJson() const;
    static UserInfo fromJson(const QJsonObject &json);
};

} // namespace UsersService

#endif // USERDATAMODELS_H
