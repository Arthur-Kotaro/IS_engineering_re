#ifndef AUTHSERVICE_H
#define AUTHSERVICE_H

#include "UsersServiceClient_global.h"
#include "UserProfile.h"
#include <QObject>
#include <memory>

namespace UsersService {

class ApiClient;

struct USERSERVICECLIENT_EXPORT LoginResponse {
    QString accessToken;
    QString refreshToken;
    QString tokenType = "bearer";
    bool requiresPasswordChange = false;
};

struct USERSERVICECLIENT_EXPORT UserSession {
    bool isValid = false;
    QString accessToken;
    QString refreshToken;
    UserProfile profile;
    QJsonObject userPayload;
};

struct USERSERVICECLIENT_EXPORT AuthResult {
    bool success = false;
    QString errorType;
    QString errorMessage;
    UserSession session;
    bool requiresPasswordChange = false;
};

class USERSERVICECLIENT_EXPORT AuthService : public QObject
{
    Q_OBJECT

public:
    explicit AuthService(std::shared_ptr<ApiClient> apiClient, QObject *parent = nullptr);
    
    void login(const QString& email, const QString& password);
    void refreshToken(const QString& refreshToken);
    void logout(const QString& refreshToken = QString());
    void changePassword(const QString& currentPassword, const QString& newPassword, 
                        const QString& confirmPassword);
    void resetPassword(const QString& email);
    void checkPasswordExpiry();
    void fetchUserProfile();
    void fetchUserStatus();
    
    UserSession currentSession() const;
    void restoreSession(const QString& accessToken, const QString& refreshToken);
    bool isSessionValid() const;
    
signals:
    void loginCompleted(const AuthResult& result);
    void tokenRefreshed(const QString& newAccessToken, const QString& newRefreshToken);
    void passwordChanged(bool success, const QString& message);
    void passwordReset(bool success, const QString& message);
    void passwordExpiryInfo(int daysRemaining, bool isExpired, const QString& expiresAt);
    void profileFetched(const UserProfile& profile);
    void statusFetched(const QJsonObject& status);
    void sessionExpired();

private:
    void parseJwtPayload(const QString& token, QJsonObject& payload);
    void updateSessionFromTokens();
    
private:
    std::shared_ptr<ApiClient> m_apiClient;
    UserSession m_currentSession;
};

} // namespace UsersService

#endif // AUTHSERVICE_H
