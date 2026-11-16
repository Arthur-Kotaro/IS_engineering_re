#include "WebSocketClient.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QNetworkRequest>

WebSocketClient::WebSocketClient(QObject *parent)
    : QObject(parent), m_ws(new QWebSocket())
{
    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setSingleShot(true);
    connect(m_reconnectTimer, &QTimer::timeout, this, &WebSocketClient::tryReconnect);

    connect(m_ws, &QWebSocket::connected, this, &WebSocketClient::onConnected);
    connect(m_ws, &QWebSocket::disconnected, this, &WebSocketClient::onDisconnected);
    connect(m_ws, &QWebSocket::textMessageReceived, this, &WebSocketClient::onTextMessage);
    connect(m_ws, &QWebSocket::errorOccurred, this, &WebSocketClient::onError);
}

WebSocketClient::~WebSocketClient()
{
    m_manualDisconnect = true;
    m_ws->close();
}

QString WebSocketClient::connectionState() const
{
    switch (m_state) {
    case Disconnected: return "disconnected";
    case Connecting: return "connecting";
    case Connected: return "connected";
    case Reconnecting: return "reconnecting";
    }
    return "unknown";
}

void WebSocketClient::setState(State state)
{
    if (m_state != state) {
        m_state = state;
        emit connectionStateChanged();
        qDebug() << "WebSocketClient: state =" << connectionState();
    }
}

void WebSocketClient::connectToServer(const QString& url, const QString& token)
{
    m_url = url;
    m_token = token;
    m_manualDisconnect = false;
    m_reconnectAttempt = 0;

    // Добавляем last_version в URL
    QString fullUrl = url;
    if (!url.contains("last_version=")) {
        fullUrl += (url.contains("?") ? "&" : "?") + QString("last_version=%1").arg(m_lastVersion);
    }

    QUrl wsUrl(fullUrl);
    QNetworkRequest request(wsUrl);

    if (!token.isEmpty()) {
        request.setRawHeader("Authorization", QString("Bearer %1").arg(token).toUtf8());
    }

    // ВАЖНО: передаём X-User-ID для прямых подключений (WS к BUS_service)
    if (m_userId > 0) {
        request.setRawHeader("X-User-ID", QByteArray::number(m_userId));
    }

    setState(Connecting);
    m_ws->open(request);
}

void WebSocketClient::disconnectFromServer()
{
    m_manualDisconnect = true;
    m_reconnectTimer->stop();
    m_ws->close();
    setState(Disconnected);
}

void WebSocketClient::sendDiff(const QJsonObject& diff)
{
    if (m_state == Connected && m_ws->isValid()) {
        m_ws->sendTextMessage(QJsonDocument(diff).toJson(QJsonDocument::Compact));
    } else {
        // Копим в очередь для отправки после реконнекта
        m_pendingDiffs.enqueue(diff);
        qDebug() << "WebSocketClient: diff queued (state =" << connectionState() << "), queue size =" << m_pendingDiffs.size();
    }
}

void WebSocketClient::flushQueue()
{
    while (!m_pendingDiffs.isEmpty() && m_state == Connected) {
        QJsonObject diff = m_pendingDiffs.dequeue();
        m_ws->sendTextMessage(QJsonDocument(diff).toJson(QJsonDocument::Compact));
    }
}

void WebSocketClient::onConnected()
{
    qDebug() << "=== WebSocketClient: connected";
    m_reconnectAttempt = 0;
    setState(Connected);
    emit connected();
    flushQueue();
}

void WebSocketClient::onDisconnected()
{
    qDebug() << "=== WebSocketClient: disconnected";
    setState(Disconnected);
    emit disconnected();

    if (!m_manualDisconnect) {
        // Автоматический реконнект
        int delay = qMin(1000 * (1 << m_reconnectAttempt), 30000);
        m_reconnectAttempt++;
        setState(Reconnecting);
        qDebug() << "WebSocketClient: reconnect in" << delay << "ms (attempt" << m_reconnectAttempt << ")";
        m_reconnectTimer->start(delay);
    }
}

void WebSocketClient::tryReconnect()
{
    if (m_manualDisconnect) return;
    qDebug() << "WebSocketClient: trying to reconnect...";
    connectToServer(m_url, m_token);
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

    if (type == "snapshot") {
        // Обновляем last_version из snapshot
        m_lastVersion = obj.value("version").toInt(m_lastVersion);
        emit snapshotReceived(obj);
    } else if (type == "diff") {
        emit diffReceived(obj);
    } else if (type == "error") {
        emit errorOccurred(obj.value("message").toString());
    } else if (type == "ack") {
        // Игнорируем ack
    }
}

void WebSocketClient::onError(QAbstractSocket::SocketError err)
{
    Q_UNUSED(err)
    qDebug() << "=== WebSocketClient error:" << m_ws->errorString();
    emit errorOccurred(m_ws->errorString());
    // onDisconnected вызовется автоматически
}
