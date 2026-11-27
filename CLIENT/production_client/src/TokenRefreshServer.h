#ifndef TOKENREFRESHSERVER_H
#define TOKENREFRESHSERVER_H

#include <QObject>
#include <QLocalServer>
#include <QList>

class QLocalSocket;

namespace UsersService {
class AuthService;
}

/**
 * Сервер IPC для refresh токена. Слушает QLocalServer с именем
 * "engineering_re_token_refresh". Дочерние процессы (gantt_editor)
 * подключаются и шлют {"action":"refresh"}.
 *
 * При получении refresh — вызывает AuthService::refreshToken(currentRefreshToken).
 * После успешного refresh (сигнал tokenRefreshed) — рассылает всем
 * подключённым клиентам {"action":"token","token":"..."}.
 */
class TokenRefreshServer : public QObject
{
    Q_OBJECT
public:
    explicit TokenRefreshServer(QObject *parent = nullptr);
    ~TokenRefreshServer();

    // Установить AuthService (для вызова refreshToken и получения currentSession)
    void setAuthService(UsersService::AuthService* authService) { m_authService = authService; }

    // Установить имя сервера (по умолчанию "engineering_re_token_refresh")
    void setServerName(const QString& name) { m_serverName = name; }

    bool listen();
    void stop();

    // Установить текущий refresh-token (обновляется из AuthService::tokenRefreshed)
    void setCurrentRefreshToken(const QString& token) { m_currentRefreshToken = token; }

public slots:
    // Вызывается из AuthService::tokenRefreshed
    void onTokenRefreshed(const QString& newAccessToken, const QString& newRefreshToken);

    // Вызывается из AuthService::sessionExpired
    void onSessionExpired();

private slots:
    void onNewConnection();
    void onClientDisconnected();
    void onClientReadyRead();

private:
    void broadcastToken(const QString& token);
    void sendError(QLocalSocket* socket, const QString& message);

    QLocalServer* m_server;
    QString m_serverName = "engineering_re_token_refresh";
    UsersService::AuthService* m_authService = nullptr;
    QList<QLocalSocket*> m_clients;
    QString m_currentRefreshToken;
};

#endif // TOKENREFRESHSERVER_H
