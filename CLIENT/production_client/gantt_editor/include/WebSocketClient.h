#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <QObject>
#include <QWebSocket>
#include <QJsonObject>

class WebSocketClient : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketClient(QObject *parent = nullptr);

    void connectToServer(const QString& url, const QString& token);
    void disconnectFromServer();
    void sendDiff(const QJsonObject& diff);

signals:
    void connected();
    void disconnected();
    void snapshotReceived(const QJsonObject& snapshot);
    void diffReceived(const QJsonObject& diff);
    void errorOccurred(const QString& message);

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessage(const QString& message);
    void onError(QAbstractSocket::SocketError err);

private:
    QWebSocket* m_ws;
    QString m_token;
};

#endif
