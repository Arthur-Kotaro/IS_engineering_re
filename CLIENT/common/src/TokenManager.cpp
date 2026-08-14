// common/src/TokenManager.cpp
#include "userserviceclient/TokenManager.h"
#include <QJsonObject>

namespace UsersService {

    TokenManager::TokenManager(QObject* parent) : QObject(parent) {}

    void TokenManager::setTokens(const QString& accessToken, const QString& refreshToken)
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

    void TokenManager::updateRefreshToken(const QString& refreshToken)
    {
        m_refreshToken = refreshToken;
        emit tokensChanged();
    }

    bool TokenManager::hasValidAccessToken() const
    {
        return !m_accessToken.isEmpty();
    }

    QJsonObject TokenManager::saveToJson() const
    {
        QJsonObject json;
        json["access_token"] = m_accessToken;
        json["refresh_token"] = m_refreshToken;
        return json;
    }

    void TokenManager::loadFromJson(const QJsonObject& json)
    {
        m_accessToken = json["access_token"].toString();
        m_refreshToken = json["refresh_token"].toString();
        emit tokensChanged();
    }

} // namespace UsersService
