// common/src/TokenManager.cpp
#include "userserviceclient/TokenManager.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

namespace UsersService
{

TokenManager::TokenManager(QObject *parent) : QObject(parent) {}

void TokenManager::setTokens(const QString &accessToken, const QString &refreshToken)
{
    m_accessToken = accessToken;
    m_refreshToken = refreshToken;
    emit tokensChanged();
}

void TokenManager::clearTokens()
{
    m_accessToken.clear();
    m_refreshToken.clear();
    emit tokensChanged();
}

void TokenManager::updateRefreshToken(const QString &refreshToken)
{
    m_refreshToken = refreshToken;
    emit tokensChanged();
}

bool TokenManager::hasValidAccessToken() const
{
    return !m_accessToken.isEmpty() && isTokenValid(m_accessToken);
}

QJsonObject TokenManager::saveToJson() const
{
    QJsonObject json;
    json["access_token"] = m_accessToken;
    json["refresh_token"] = m_refreshToken;
    return json;
}

void TokenManager::loadFromJson(const QJsonObject &json)
{
    m_accessToken = json["access_token"].toString();
    m_refreshToken = json["refresh_token"].toString();
    emit tokensChanged();
}

QDateTime TokenManager::getTokenExpiry(const QString &token) const
{
    if (token.isEmpty())
        return QDateTime();

    QStringList parts = token.split('.');
    if (parts.size() != 3)
        return QDateTime();

    // Декодируем payload (Base64Url → Base64)
    QByteArray payloadB64 = parts[1].toUtf8();
    payloadB64.replace('-', '+').replace('_', '/');
    while (payloadB64.size() % 4)
        payloadB64.append('=');

    QByteArray payloadJson = QByteArray::fromBase64(payloadB64);
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(payloadJson, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
        return QDateTime();

    QJsonObject payload = doc.object();
    if (!payload.contains("exp"))
        return QDateTime();

    qint64 exp = static_cast<qint64>(payload["exp"].toDouble());
    return QDateTime::fromSecsSinceEpoch(exp, Qt::UTC);
}

bool TokenManager::isTokenValid(const QString &token) const
{
    if (token.isEmpty())
        return false;

    QDateTime expiry = getTokenExpiry(token);
    if (!expiry.isValid())
        return false; // не удалось декодировать — считаем невалидным

    // Добавляем 30 секунд запаса (чтобы не отправить токен, который истечёт в пути)
    QDateTime now = QDateTime::currentDateTimeUtc().addSecs(30);
    return now < expiry;
}

} // namespace UsersService
