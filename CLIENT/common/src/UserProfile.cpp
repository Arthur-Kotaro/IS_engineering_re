#include "userserviceclient/UserProfile.h"
#include <QJsonArray>

namespace UsersService {

QString UserProfile::fullName() const {
    QStringList parts;
    if (!lastName.isEmpty()) parts << lastName;
    if (!firstName.isEmpty()) parts << firstName;
    if (!middleName.isEmpty()) parts << middleName;
    return parts.join(" ");
}

QJsonObject UserProfile::toJson() const {
    QJsonObject json;
    json["user_id"] = userId;
    json["user_name"] = userName;
    json["email"] = email;
    json["last_name"] = lastName;
    json["first_name"] = firstName;
    json["middle_name"] = middleName;
    json["full_name"] = fullName();
    json["gender"] = gender;
    json["birth_date"] = birthDate.toString(Qt::ISODate);
    json["position_id"] = positionId;
    json["position_code"] = positionCode;
    json["position_name_ru"] = positionNameRu;
    json["dept_id"] = deptId;
    json["dept_code"] = deptCode;
    json["dept_name_ru"] = deptNameRu;
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
    json["is_super_admin"] = isSuperAdmin;

    QJsonArray rolesArray;
    for (const auto& r : roles) rolesArray.append(r);
    json["roles"] = rolesArray;

    QJsonArray roleCodesArray;
    for (const auto& r : roleCodes) roleCodesArray.append(r);
    json["role_codes"] = roleCodesArray;

    return json;
}

UserProfile UserProfile::fromJson(const QJsonObject& json) {
    UserProfile p;
    p.userId = json["user_id"].toInt();
    p.userName = json["user_name"].toString();
    p.email = json["email"].toString();
    p.lastName = json["last_name"].toString();
    p.firstName = json["first_name"].toString();
    p.middleName = json["middle_name"].toString();
    p.gender = json["gender"].toString();
    p.birthDate = QDateTime::fromString(json["birth_date"].toString(), Qt::ISODate);
    p.positionId = json["position_id"].toInt();
    p.positionCode = json["position_code"].toString();
    p.positionNameRu = json["position_name_ru"].toString();
    p.deptId = json["dept_id"].toString();
    p.deptCode = json["dept_code"].toString();
    p.deptNameRu = json["dept_name_ru"].toString();
    p.phoneWork = json["phone_work"].toString();
    p.phoneMobile = json["phone_mobile"].toString();
    p.status = json["status"].toString();
    p.isBlocked = json["is_blocked"].toBool();
    p.isDeleted = json["is_deleted"].toBool();
    p.blockedReason = json["blocked_reason"].toString();
    p.blockedExpiresAt = QDateTime::fromString(json["block_expires_at"].toString(), Qt::ISODate);
    p.deletedAt = QDateTime::fromString(json["deleted_at"].toString(), Qt::ISODate);
    p.createdAt = QDateTime::fromString(json["created_at"].toString(), Qt::ISODate);
    p.updatedAt = QDateTime::fromString(json["updated_at"].toString(), Qt::ISODate);
    p.lastLoginAt = QDateTime::fromString(json["last_login_at"].toString(), Qt::ISODate);
    p.passwordUpdatedAt = QDateTime::fromString(json["password_updated_at"].toString(), Qt::ISODate);
    p.isSuperAdmin = json["is_super_admin"].toBool();

    for (const auto& v : json["roles"].toArray()) p.roles.append(v.toString());
    for (const auto& v : json["role_codes"].toArray()) p.roleCodes.append(v.toString());

    return p;
}

} // namespace UsersService
