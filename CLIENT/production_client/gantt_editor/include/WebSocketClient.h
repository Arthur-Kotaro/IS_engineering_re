#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <QObject>
#include <QWebSocket>
#include <QJsonObject>
#include <QQueue>
#include <QTimer>
#include <functional>

class WebSocketClient : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketClient(QObject *parent = nullptr);
    ~WebSocketClient();

    void connectToServer(const QString& url, const QString& token);
    void disconnectFromServer();
    void sendDiff(const QJsonObject& diff);

    // Установка ID пользователя для прямых подключений (WS к BUS)
    void setUserId(int userId) { m_userId = userId; }

    // Callback для refresh токена (аналогично ApiClient).
    // Вызывается при 401 / "Token expired" / close code 1008.
    // Возвращает true, если refresh запрошен; после успешного refresh
    // клиент должен вызвать onTokenRefreshed(newToken) — переподключение.
    using TokenExpiredHandler = std::function<bool()>;
    void setOnTokenExpired(TokenExpiredHandler handler) { m_onTokenExpired = std::move(handler); }
    void onTokenRefreshed(const QString& newToken);
    void onTokenRefreshFailed();

    // Индикатор состояния
    Q_PROPERTY(QString connectionState READ connectionState NOTIFY connectionStateChanged)

    enum State {
        Disconnected = 0,
        Connecting,
        Connected,
        Reconnecting
    };
    Q_ENUM(State)

    State state() const { return m_state; }
    QString connectionState() const;

    // Установка версии для last_version
    void setLastVersion(int version) { m_lastVersion = version; }

signals:
    void connected();
    void disconnected();
    void snapshotReceived(const QJsonObject& snapshot);
    void diffReceived(const QJsonObject& diff);
    void errorOccurred(const QString& message);
    void connectionStateChanged();

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessage(const QString& message);
    void onError(QAbstractSocket::SocketError err);
    void tryReconnect();

private:
    void setState(State state);
    void flushQueue();

    QWebSocket* m_ws;
    QString m_url;
    QString m_token;
    int m_userId = 0;
    QQueue<QJsonObject> m_pendingDiffs;
    QTimer* m_reconnectTimer;
    int m_reconnectAttempt = 0;
    int m_lastVersion = 0;
    State m_state = Disconnected;
    bool m_manualDisconnect = false;

    TokenExpiredHandler m_onTokenExpired;
    bool m_refreshing = false;
    bool m_reconnectAfterRefresh = false;
};

#endif // WEBSOCKETCLIENT_H
