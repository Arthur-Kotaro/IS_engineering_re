#include "WebSocketClient.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

WebSocketClient::WebSocketClient(QObject *parent)
    : QObject(parent), m_ws(new QWebSocket())
{
    connect(m_ws, &QWebSocket::connected, this, &WebSocketClient::onConnected);
    connect(m_ws, &QWebSocket::disconnected, this, &WebSocketClient::onDisconnected);
    connect(m_ws, &QWebSocket::textMessageReceived, this, &WebSocketClient::onTextMessage);
    connect(m_ws, &QWebSocket::errorOccurred, this, &WebSocketClient::onError);
}

void WebSocketClient::connectToServer(const QString& url, const QString& token)
{
    m_token = token;
    QUrl wsUrl(url);
    QNetworkRequest request(wsUrl);
    if (!token.isEmpty())
        request.setRawHeader("Authorization", QString("Bearer %1").arg(token).toUtf8());
    m_ws->open(request);
}

void WebSocketClient::disconnectFromServer()
{
    m_ws->close();
}

void WebSocketClient::sendDiff(const QJsonObject& diff)
{
    if (m_ws->isValid())
        m_ws->sendTextMessage(QJsonDocument(diff).toJson(QJsonDocument::Compact));
}

void WebSocketClient::onConnected()
{
    qDebug() << "=== WebSocketClient: connected";
    emit connected();
}

void WebSocketClient::onDisconnected()
{
    qDebug() << "=== WebSocketClient: disconnected";
    emit disconnected();
}

void WebSocketClient::onTextMessage(const QString& message)
{
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        qDebug() << "=== WebSocketClient: invalid JSON";
        return;
    }
    QJsonObject obj = doc.object();
    QString type = obj.value("type").toString();
    qDebug() << "=== WebSocketClient: message type =" << type;
    if (type == "snapshot")
        emit snapshotReceived(obj);
    else if (type == "diff")
        emit diffReceived(obj);
    else if (type == "error")
        emit errorOccurred(obj.value("message").toString());
}

void WebSocketClient::onError(QAbstractSocket::SocketError err)
{
    Q_UNUSED(err)
    qDebug() << "=== WebSocketClient error:" << m_ws->errorString();
    emit errorOccurred(m_ws->errorString());
}
