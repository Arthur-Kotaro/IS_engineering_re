#include "userserviceclient/UserDataModels.h"
#include <QJsonArray>

namespace UsersService {

QJsonObject ProjectRole::toJson() const {
    QJsonObject json;
    json["project_id"] = projectId;
    json["project_name"] = projectName;
    json["role"] = role;
    json["last_modified"] = lastModified.toString(Qt::ISODate);
    return json;
}

ProjectRole ProjectRole::fromJson(const QJsonObject& json) {
    ProjectRole role;
    role.projectId = json["project_id"].toInt();
    role.projectName = json["project_name"].toString();
    role.role = json["role"].toString();
    role.lastModified = QDateTime::fromString(json["last_modified"].toString(), Qt::ISODate);
    return role;
}

QJsonObject UserInfo::toJson() const {
    QJsonObject json;
    json["user_id"] = userId;
    json["email"] = email;
    json["full_name"] = fullName;
    json["position"] = position;
    json["password_expires_at"] = passwordExpiresAt.toString(Qt::ISODate);
    
    QJsonArray rolesArray;
    for (const auto& role : projectRoles) {
        rolesArray.append(role.toJson());
    }
    json["project_roles"] = rolesArray;
    
    return json;
}

UserInfo UserInfo::fromJson(const QJsonObject& json) {
    UserInfo info;
    info.userId = json["user_id"].toString();
    info.email = json["email"].toString();
    info.fullName = json["full_name"].toString();
    info.position = json["position"].toString();
    info.passwordExpiresAt = QDateTime::fromString(json["password_expires_at"].toString(), Qt::ISODate);
    
    QJsonArray rolesArray = json["project_roles"].toArray();
    for (const auto& roleJson : rolesArray) {
        info.projectRoles.append(ProjectRole::fromJson(roleJson.toObject()));
    }
    
    return info;
}

} // namespace UsersService
