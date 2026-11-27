#include "TokenRefreshClient.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

TokenRefreshClient::TokenRefreshClient(QObject *parent)
    : QObject(parent)
    , m_socket(new QLocalSocket(this))
    , m_reconnectTimer(new QTimer(this))
{
    m_reconnectTimer->setSingleShot(true);
    connect(m_reconnectTimer, &QTimer::timeout, this, &TokenRefreshClient::tryReconnect);

    connect(m_socket, &QLocalSocket::connected, this, &TokenRefreshClient::onConnected);
    connect(m_socket, &QLocalSocket::disconnected, this, &TokenRefreshClient::onDisconnected);
    connect(m_socket, &QLocalSocket::readyRead, this, &TokenRefreshClient::onReadyRead);
    connect(m_socket, &QLocalSocket::errorOccurred, this, &TokenRefreshClient::onError);

    // Пытаемся подключиться сразу (не блокируя)
    m_socket->connectToServer(m_serverName);
}

TokenRefreshClient::~TokenRefreshClient()
{
    m_socket->disconnectFromServer();
}

bool TokenRefreshClient::isConnected() const
{
    return m_socket->state() == QLocalSocket::ConnectedState;
}

bool TokenRefreshClient::requestRefresh()
{
    qDebug() << "[TokenRefreshClient] requestRefresh, socket state =" << m_socket->state();
    m_pendingRefresh = true;

    if (isConnected()) {
        sendRequest();
        return true;
    }

    // Не подключены — пробуем подключиться, запрос отправим в onConnected
    if (m_socket->state() == QLocalSocket::UnconnectedState) {
        m_socket->connectToServer(m_serverName);
    }
    // Если Connecting — ждём onConnected
    return true;
}

void TokenRefreshClient::onConnected()
{
    qDebug() << "[TokenRefreshClient] connected to server";
    if (m_pendingRefresh) {
        sendRequest();
    }
}

void TokenRefreshClient::onDisconnected()
{
    qDebug() << "[TokenRefreshClient] disconnected";
    if (m_pendingRefresh) {
        // Не удалось — повтор через 1 сек
        m_reconnectTimer->start(1000);
    }
}

void TokenRefreshClient::onReadyRead()
{
    m_buffer += m_socket->readAll();
    int idx;
    while ((idx = m_buffer.indexOf('\n')) >= 0) {
        QByteArray line = m_buffer.left(idx);
        m_buffer.remove(0, idx + 1);
        if (!line.isEmpty())
            processLine(line);
    }
}

void TokenRefreshClient::onError(QLocalSocket::LocalSocketError err)
{
    Q_UNUSED(err)
    qDebug() << "[TokenRefreshClient] socket error:" << m_socket->errorString();
    if (m_pendingRefresh) {
        m_reconnectTimer->start(1000);
    }
}

void TokenRefreshClient::tryReconnect()
{
    if (!m_pendingRefresh) return;
    if (m_socket->state() == QLocalSocket::UnconnectedState) {
        m_socket->connectToServer(m_serverName);
    }
}

void TokenRefreshClient::sendRequest()
{
    QJsonObject req;
    req["action"] = "refresh";
    QByteArray data = QJsonDocument(req).toJson(QJsonDocument::Compact) + "\n";
    m_socket->write(data);
    m_socket->flush();
    qDebug() << "[TokenRefreshClient] sent refresh request";
}

void TokenRefreshClient::processLine(const QByteArray& line)
{
    QJsonDocument doc = QJsonDocument::fromJson(line);
    if (!doc.isObject()) {
        qDebug() << "[TokenRefreshClient] invalid JSON:" << line;
        return;
    }
    QJsonObject obj = doc.object();
    QString action = obj.value("action").toString();

    if (action == "token") {
        QString token = obj.value("token").toString();
        qDebug() << "[TokenRefreshClient] got new token, length =" << token.length();
        m_pendingRefresh = false;
        emit tokenRefreshed(token);
    } else if (action == "error") {
        QString msg = obj.value("message").toString();
        qDebug() << "[TokenRefreshClient] refresh failed:" << msg;
        m_pendingRefresh = false;
        emit refreshFailed(msg);
    }
}
