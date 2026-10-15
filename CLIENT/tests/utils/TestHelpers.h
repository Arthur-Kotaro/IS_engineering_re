#ifndef TESTHELPERS_H
#define TESTHELPERS_H

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

namespace TestHelpers
{

// Генерация JWT токена для тестов
QString generateTestJwtToken(const QJsonObject &payload);

// Создание тестового payload пользователя
QJsonObject createTestUserPayload();

// Создание ответа сервера для тестов
QJsonObject createLoginResponse(const QString &accessToken, const QString &refreshToken);

} // namespace TestHelpers

#endif // TESTHELPERS_H
