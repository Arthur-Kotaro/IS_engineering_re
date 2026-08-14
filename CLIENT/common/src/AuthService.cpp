// common/src/AuthService.cpp
#include "userserviceclient/AuthService.h"
#include "userserviceclient/ApiClient.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>

namespace UsersService {

    AuthService::AuthService(std::shared_ptr<ApiClient> apiClient, QObject* parent)
    : QObject(parent)
    , m_apiClient(apiClient)
    {
    }

    void AuthService::parseJwtPayload(const QString& token, QJsonObject& payload) {
        QStringList parts = token.split('.');
        if (parts.size() >= 2) {
            QString payloadBase64 = parts[1];
            while (payloadBase64.length() % 4) {
                payloadBase64 += '=';
            }
            QByteArray payloadJson = QByteArray::fromBase64(payloadBase64.toLatin1());
            QJsonDocument doc = QJsonDocument::fromJson(payloadJson);
            payload = doc.object();
        }
    }

    void AuthService::login(const QString& email, const QString& password) {
        QJsonObject loginData;
        loginData["email"] = email;
        loginData["password"] = password;

        qDebug() << "Login request to:" << email;

        // Исправлено: правильный путь через Gateway
        m_apiClient->post("/login", loginData,
                          [this, email](const ApiResponse& response) {
                              AuthResult result;

                              if (!response.success) {
                                  result.success = false;
                                  result.errorType = response.errorType;
                                  result.errorMessage = response.errorString;
                                  emit loginCompleted(result);
                                  return;
                              }

                              QJsonObject data = response.data.object();
                              result.success = true;
                              result.session.accessToken = data["access_token"].toString();
                              result.session.refreshToken = data["refresh_token"].toString();
                              result.requiresPasswordChange = data["requires_password_change"].toBool();

                              qDebug() << "Login successful, access token received";

                              m_apiClient->setAuthToken(result.session.accessToken);
                              m_currentSession = result.session;

                              // Парсим JWT для получения user_id и ролей
                              parseJwtPayload(result.session.accessToken, m_currentSession.userPayload);
                              updateSessionFromTokens();

                              // Получаем профиль пользователя
                              fetchUserProfile();
                              checkPasswordExpiry();

                              emit loginCompleted(result);
                          });
    }

    void AuthService::fetchUserProfile() {
        qDebug() << "Fetching user profile...";

        // Исправлено: правильный путь
        m_apiClient->get("/users/me",
                         [this](const ApiResponse& response) {
                             if (response.success) {
                                 QJsonObject data = response.data.object();
                                 m_currentSession.profile = UserProfile::fromJson(data);
                                 qDebug() << "Profile fetched:" << m_currentSession.profile.userName;
                                 qDebug() << "Email:" << m_currentSession.profile.email;
                                 qDebug() << "Roles:" << m_currentSession.profile.roles;

                                 emit profileFetched(m_currentSession.profile);
                             } else {
                                 qDebug() << "Failed to fetch profile:" << response.errorString;
                             }
                         });
    }

    void AuthService::checkPasswordExpiry() {
        qDebug() << "Checking password expiry...";

        // Исправлено: правильный путь
        m_apiClient->get("/password-expiry",
                         [this](const ApiResponse& response) {
                             if (response.success) {
                                 QJsonObject data = response.data.object();
                                 int daysRemaining = data["days_remaining"].toInt();
                                 bool isExpired = data["is_expired"].toBool();
                                 QString expiresAt = data["expires_at"].toString();
                                 qDebug() << "Password expiry:" << daysRemaining << "days left";
                                 emit passwordExpiryInfo(daysRemaining, isExpired, expiresAt);
                             } else {
                                 qDebug() << "Failed to get password expiry:" << response.errorString;
                             }
                         });
    }

    void AuthService::refreshToken(const QString& refreshToken) {
        QJsonObject refreshData;
        refreshData["refresh_token"] = refreshToken;

        // Исправлено: правильный путь
        m_apiClient->post("/refresh", refreshData,
                          [this](const ApiResponse& response) {
                              if (response.success) {
                                  QJsonObject data = response.data.object();
                                  emit tokenRefreshed(data["access_token"].toString(), data["refresh_token"].toString());
                              }
                          });
    }

    void AuthService::logout(const QString& refreshToken) {
        QJsonObject logoutData;
        if (!refreshToken.isEmpty()) {
            logoutData["refresh_token"] = refreshToken;
        }

        // Исправлено: правильный путь
        m_apiClient->post("/logout", logoutData,
                          [this](const ApiResponse& response) {
                              m_currentSession = UserSession();
                              m_apiClient->clearAuthToken();
                              Q_UNUSED(response);
                          });
    }

    void AuthService::changePassword(const QString& currentPassword, const QString& newPassword,
                                     const QString& confirmPassword) {
        QJsonObject changeData;
        changeData["current_password"] = currentPassword;
        changeData["new_password"] = newPassword;
        changeData["confirm_password"] = confirmPassword;

        // Исправлено: правильный путь
        m_apiClient->post("/change-password", changeData,
                          [this](const ApiResponse& response) {
                              if (response.success) {
                                  QJsonObject data = response.data.object();
                                  emit passwordChanged(true, data["message"].toString());
                                  emit sessionExpired();
                              } else {
                                  emit passwordChanged(false, response.errorString);
                              }
                          });
                                     }

                                     void AuthService::resetPassword(const QString& email) {
                                         QJsonObject resetData;
                                         resetData["email"] = email;

                                         // Исправлено: правильный путь
                                         m_apiClient->post("/reset-password", resetData,
                                                           [this](const ApiResponse& response) {
                                                               if (response.success) {
                                                                   QJsonObject data = response.data.object();
                                                                   emit passwordReset(true, data["message"].toString());
                                                               } else {
                                                                   emit passwordReset(false, response.errorString);
                                                               }
                                                           });
                                     }

                                     void AuthService::fetchUserStatus() {
                                         // Исправлено: правильный путь
                                         m_apiClient->get("/users/me/status",
                                                          [this](const ApiResponse& response) {
                                                              if (response.success) {
                                                                  emit statusFetched(response.data.object());
                                                              }
                                                          });
                                     }

                                     UserSession AuthService::currentSession() const {
                                         return m_currentSession;
                                     }

                                     void AuthService::restoreSession(const QString& accessToken, const QString& refreshToken) {
                                         m_currentSession.accessToken = accessToken;
                                         m_currentSession.refreshToken = refreshToken;
                                         m_currentSession.isValid = true;
                                         m_apiClient->setAuthToken(accessToken);
                                         parseJwtPayload(accessToken, m_currentSession.userPayload);
                                         updateSessionFromTokens();
                                         fetchUserProfile();
                                     }

                                     bool AuthService::isSessionValid() const {
                                         return m_currentSession.isValid && !m_currentSession.accessToken.isEmpty();
                                     }

                                     void AuthService::updateSessionFromTokens() {
                                         const QJsonObject& payload = m_currentSession.userPayload;
                                         m_currentSession.profile.userId = payload["user_id"].toInt();
                                         m_currentSession.profile.userName = payload["user_name"].toString();
                                         m_currentSession.profile.email = payload["email"].toString();

                                         QJsonArray rolesArray = payload["roles"].toArray();
                                         m_currentSession.profile.roles.clear();
                                         for (const auto& role : rolesArray) {
                                             m_currentSession.profile.roles.append(role.toString());
                                         }
                                     }

} // namespace UsersService
