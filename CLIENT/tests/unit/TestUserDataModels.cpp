#include <gtest/gtest.h>
#include "userserviceclient/UserDataModels.h"

using namespace UsersService;

TEST(UserDataModelsTest, ProjectRoleToJson) {
    ProjectRole role;
    role.projectId = 123;
    role.projectName = "Test Project";
    role.role = "Developer";
    role.lastModified = QDateTime::fromString("2024-01-01T12:00:00", Qt::ISODate);
    
    QJsonObject json = role.toJson();
    
    EXPECT_EQ(json["project_id"].toInt(), 123);
    EXPECT_EQ(json["project_name"].toString().toStdString(), "Test Project");
    EXPECT_EQ(json["role"].toString().toStdString(), "Developer");
}

TEST(UserDataModelsTest, ProjectRoleFromJson) {
    QJsonObject json;
    json["project_id"] = 456;
    json["project_name"] = "Another Project";
    json["role"] = "QA";
    json["last_modified"] = "2024-02-01T15:30:00";
    
    ProjectRole role = ProjectRole::fromJson(json);
    
    EXPECT_EQ(role.projectId, 456);
    EXPECT_EQ(role.projectName.toStdString(), "Another Project");
    EXPECT_EQ(role.role.toStdString(), "QA");
}

TEST(UserDataModelsTest, UserInfoToJson) {
    UserInfo user;
    user.userId = "user_001";
    user.email = "john@example.com";
    user.fullName = "John Doe";
    user.position = "Manager";
    
    ProjectRole role;
    role.projectId = 1;
    role.projectName = "Project X";
    role.role = "Lead";
    user.projectRoles.append(role);
    
    QJsonObject json = user.toJson();
    
    EXPECT_EQ(json["user_id"].toString().toStdString(), "user_001");
    EXPECT_EQ(json["email"].toString().toStdString(), "john@example.com");
    EXPECT_EQ(json["full_name"].toString().toStdString(), "John Doe");
    EXPECT_EQ(json["position"].toString().toStdString(), "Manager");
    EXPECT_TRUE(json.contains("project_roles"));
    EXPECT_EQ(json["project_roles"].toArray().size(), 1);
}

TEST(UserDataModelsTest, UserInfoFromJson) {
    QJsonObject json;
    json["user_id"] = "user_002";
    json["email"] = "jane@example.com";
    json["full_name"] = "Jane Smith";
    json["position"] = "Architect";
    json["password_expires_at"] = "2025-12-31T23:59:59";
    
    QJsonArray roles;
    QJsonObject roleJson;
    roleJson["project_id"] = 10;
    roleJson["project_name"] = "Project Z";
    roleJson["role"] = "Designer";
    roles.append(roleJson);
    json["project_roles"] = roles;
    
    UserInfo user = UserInfo::fromJson(json);
    
    EXPECT_EQ(user.userId.toStdString(), "user_002");
    EXPECT_EQ(user.email.toStdString(), "jane@example.com");
    EXPECT_EQ(user.fullName.toStdString(), "Jane Smith");
    EXPECT_EQ(user.position.toStdString(), "Architect");
    EXPECT_EQ(user.projectRoles.size(), 1);
    EXPECT_EQ(user.projectRoles[0].projectName.toStdString(), "Project Z");
}
