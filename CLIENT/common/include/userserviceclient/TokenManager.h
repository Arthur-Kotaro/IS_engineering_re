#ifndef TOKENMANAGER_H
#define TOKENMANAGER_H

#include "UsersServiceClient_global.h"
#include <QObject>
#include <QString>
#include <QJsonObject>

namespace UsersService {

class USERSERVICECLIENT_EXPORT TokenManager : public QObject
{
    Q_OBJECT

public:
    explicit TokenManager(QObject *parent = nullptr);
    
    void setTokens(const QString& accessToken, const QString& refreshToken);
    void clearTokens();
    void updateRefreshToken(const QString& refreshToken);
    
    QString getAccessToken() const { return m_accessToken; }
    QString getRefreshToken() const { return m_refreshToken; }
    bool hasValidAccessToken() const;
    
    QJsonObject saveToJson() const;
    void loadFromJson(const QJsonObject& json);
    
signals:
    void tokensChanged();
    void tokenExpired();

private:
    bool isTokenValid(const QString& token) const;
    
private:
    QString m_accessToken;
    QString m_refreshToken;
};

} // namespace UsersService

#endif // TOKENMANAGER_H
