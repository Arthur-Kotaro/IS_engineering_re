#ifndef TESTHELPERS_H
#define TESTHELPERS_H

#include <QString>
#include <QJsonObject>
#include <QJsonDocument>
#include <QByteArray>

namespace TestHelpers {

// Генерация JWT токена для тестов (только для тестирования, не для продакшена!)
inline QString generateTestJwtToken(const QJsonObject& payload) {
    QByteArray header = QJsonDocument(QJsonObject{{"alg", "HS256"}, {"typ", "JWT"}}).toJson(QJsonDocument::Compact);
    QByteArray payloadBytes = QJsonDocument(payload).toJson(QJsonDocument::Compact);
    
    QString token = header.toBase64() + "." + payloadBytes.toBase64() + ".signature";
    return token;
}

// Создание тестового payload пользователя
inline QJsonObject createTestUserPayload() {
    QJsonObject payload;
    payload["sub"] = "12345";
    payload["email"] = "test@example.com";
    payload["full_name"] = "Test User";
    payload["position"] = "Engineer";
    payload["password_expires_at"] = "2025-12-31T23:59:59Z";
    
    QJsonArray projects;
    QJsonObject project1;
    project1["project_id"] = 1;
    project1["name"] = "Project Alpha";
    project1["role"] = "Developer";
    projects.append(project1);
    
    QJsonObject project2;
    project2["project_id"] = 2;
    project2["name"] = "Project Beta";
    project2["role"] = "Reviewer";
    projects.append(project2);
    
    payload["projects"] = projects;
    
    return payload;
}

// Создание ответа сервера для тестов
inline QJsonObject createLoginResponse(const QString& accessToken, const QString& refreshToken) {
    QJsonObject response;
    response["access_token"] = accessToken;
    response["refresh_token"] = refreshToken;
    return response;
}

} // namespace TestHelpers

#endif // TESTHELPERS_H
