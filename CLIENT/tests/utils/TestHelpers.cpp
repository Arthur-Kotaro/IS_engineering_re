#include "TestHelpers.h"
#include <QCryptographicHash>

namespace TestHelpers {

QString generateTestJwtToken(const QJsonObject& payload) {
    QByteArray header = QJsonDocument(QJsonObject{{"alg", "HS256"}, {"typ", "JWT"}})
                        .toJson(QJsonDocument::Compact);
    QByteArray payloadBytes = QJsonDocument(payload).toJson(QJsonDocument::Compact);
    
    QString token = header.toBase64() + "." + payloadBytes.toBase64() + ".signature";
    return token;
}

QJsonObject createTestUserPayload() {
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

QJsonObject createLoginResponse(const QString& accessToken, const QString& refreshToken) {
    QJsonObject response;
    response["access_token"] = accessToken;
    response["refresh_token"] = refreshToken;
    return response;
}

} // namespace TestHelpers
