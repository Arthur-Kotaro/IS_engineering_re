#include "userserviceclient/UserProfile.h"
#include <QJsonArray>

namespace UsersService {

QJsonObject UserProfile::toJson() const {
    QJsonObject json;
    json["user_id"] = userId;
    json["user_name"] = userName;
    json["email"] = email;
    json["gender"] = gender;
    json["birth_date"] = birthDate.toString(Qt::ISODate);
    json["dept_code"] = deptCode;
    json["phone_work"] = phoneWork;
    json["phone_mobile"] = phoneMobile;
    json["status"] = status;
    json["is_blocked"] = isBlocked;
    json["is_deleted"] = isDeleted;
    json["blocked_reason"] = blockedReason;
    json["block_expires_at"] = blockedExpiresAt.toString(Qt::ISODate);
    json["deleted_at"] = deletedAt.toString(Qt::ISODate);
    json["created_at"] = createdAt.toString(Qt::ISODate);
    json["updated_at"] = updatedAt.toString(Qt::ISODate);
    json["last_login_at"] = lastLoginAt.toString(Qt::ISODate);
    json["password_updated_at"] = passwordUpdatedAt.toString(Qt::ISODate);
    
    QJsonArray rolesArray;
    for (const QString& role : roles) {
        rolesArray.append(role);
    }
    json["roles"] = rolesArray;
    
    QJsonArray projectsArray;
    for (const QJsonObject& proj : projects) {
        projectsArray.append(proj);
    }
    json["projects"] = projectsArray;
    
    return json;
}

UserProfile UserProfile::fromJson(const QJsonObject& json) {
    UserProfile profile;
    profile.userId = json["user_id"].toInt();
    profile.userName = json["user_name"].toString();
    profile.email = json["email"].toString();
    profile.gender = json["gender"].toString();
    profile.birthDate = QDateTime::fromString(json["birth_date"].toString(), Qt::ISODate);
    profile.deptCode = json["dept_code"].toString();
    profile.phoneWork = json["phone_work"].toString();
    profile.phoneMobile = json["phone_mobile"].toString();
    profile.status = json["status"].toString();
    profile.isBlocked = json["is_blocked"].toBool();
    profile.isDeleted = json["is_deleted"].toBool();
    profile.blockedReason = json["blocked_reason"].toString();
    profile.blockedExpiresAt = QDateTime::fromString(json["block_expires_at"].toString(), Qt::ISODate);
    profile.deletedAt = QDateTime::fromString(json["deleted_at"].toString(), Qt::ISODate);
    profile.createdAt = QDateTime::fromString(json["created_at"].toString(), Qt::ISODate);
    profile.updatedAt = QDateTime::fromString(json["updated_at"].toString(), Qt::ISODate);
    profile.lastLoginAt = QDateTime::fromString(json["last_login_at"].toString(), Qt::ISODate);
    profile.passwordUpdatedAt = QDateTime::fromString(json["password_updated_at"].toString(), Qt::ISODate);
    
    QJsonArray rolesArray = json["roles"].toArray();
    for (const auto& role : rolesArray) {
        profile.roles.append(role.toString());
    }
    
    QJsonArray projectsArray = json["projects"].toArray();
    for (const auto& proj : projectsArray) {
        profile.projects.append(proj.toObject());
    }
    
    return profile;
}

QString UserProfile::getWidgetsPath() const {
    if (roles.contains("chief_project_engineer") || roles.contains("cve") || roles.contains("dcve")) {
        return "widgets/roles/chief_project_engineer/";
    }
    if (roles.contains("project_secretary") || roles.contains("ipp")) {
        return "widgets/roles/project_secretary/";
    }
    if (roles.contains("project_economist")) {
        return "widgets/roles/project_economist/";
    }
    if (roles.contains("system_architect")) {
        return "widgets/roles/system_architect/";
    }
    if (roles.contains("validator")) {
        return "widgets/roles/validator/";
    }
    if (roles.contains("test_specialist")) {
        return "widgets/roles/test_specialist/";
    }
    if (roles.contains("project_manager_ist") || roles.contains("ist")) {
        return "widgets/roles/project_manager_ist/";
    }
    if (roles.contains("project_manager_pfe") || roles.contains("pfe")) {
        return "widgets/roles/project_manager_pfe/";
    }
    if (roles.contains("project_manager_prototypes") || roles.contains("cpm")) {
        return "widgets/roles/prototype/";
    }
    return "widgets/common/";
}

} // namespace UsersService
