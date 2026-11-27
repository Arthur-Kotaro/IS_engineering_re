#ifndef TOKENREFRESHCLIENT_H
#define TOKENREFRESHCLIENT_H

#include <QObject>
#include <QLocalSocket>
#include <QJsonObject>
#include <QTimer>

/**
 * Клиент IPC для запроса refresh токена у родительского процесса
 * (production_client). Подключается к QLocalServer с именем
 * "engineering_re_token_refresh".
 *
 * Протокол (JSON-строки, разделитель \n):
 *   → {"action":"refresh"}
 *   ← {"action":"token","token":"<new_access_token>"}
 *   ← {"action":"error","message":"..."}
 */
class TokenRefreshClient : public QObject
{
    Q_OBJECT
public:
    explicit TokenRefreshClient(QObject *parent = nullptr);
    ~TokenRefreshClient();

    // Установить имя сервера (по умолчанию "engineering_re_token_refresh")
    void setServerName(const QString& name) { m_serverName = name; }

    // Запросить refresh. Возвращает true, если запрос отправлен.
    // Результат — сигнал tokenRefreshed / refreshFailed.
    bool requestRefresh();

    bool isConnected() const;

signals:
    void tokenRefreshed(const QString& newToken);
    void refreshFailed(const QString& message);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onError(QLocalSocket::LocalSocketError err);
    void tryReconnect();

private:
    void sendRequest();
    void processLine(const QByteArray& line);

    QLocalSocket* m_socket;
    QString m_serverName = "engineering_re_token_refresh";
    QTimer* m_reconnectTimer;
    bool m_pendingRefresh = false;
    QByteArray m_buffer;
};

#endif // TOKENREFRESHCLIENT_H
