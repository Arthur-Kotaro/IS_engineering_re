#include "TokenRefreshServer.h"
#include "userserviceclient/AuthService.h"
#include <QLocalServer>
#include <QLocalSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

TokenRefreshServer::TokenRefreshServer(QObject *parent)
    : QObject(parent)
    , m_server(new QLocalServer(this))
{
    connect(m_server, &QLocalServer::newConnection, this, &TokenRefreshServer::onNewConnection);
}

TokenRefreshServer::~TokenRefreshServer()
{
    stop();
}

bool TokenRefreshServer::listen()
{
    // Удаляем старый сокет (если был) — иначе listen() может упасть
    QLocalServer::removeServer(m_serverName);
    if (!m_server->listen(m_serverName)) {
        qWarning() << "[TokenRefreshServer] listen failed:" << m_server->errorString();
        return false;
    }
    qDebug() << "[TokenRefreshServer] listening on" << m_serverName;
    return true;
}

void TokenRefreshServer::stop()
{
    m_server->close();
    for (auto* c : m_clients) {
        c->disconnectFromServer();
        c->deleteLater();
    }
    m_clients.clear();
}

void TokenRefreshServer::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QLocalSocket* socket = m_server->nextPendingConnection();
        m_clients.append(socket);
        connect(socket, &QLocalSocket::disconnected, this, &TokenRefreshServer::onClientDisconnected);
        connect(socket, &QLocalSocket::readyRead, this, &TokenRefreshServer::onClientReadyRead);
        qDebug() << "[TokenRefreshServer] client connected, total =" << m_clients.size();
    }
}

void TokenRefreshServer::onClientDisconnected()
{
    QLocalSocket* socket = qobject_cast<QLocalSocket*>(sender());
    if (socket) {
        m_clients.removeAll(socket);
        socket->deleteLater();
    }
    qDebug() << "[TokenRefreshServer] client disconnected, total =" << m_clients.size();
}

void TokenRefreshServer::onClientReadyRead()
{
    QLocalSocket* socket = qobject_cast<QLocalSocket*>(sender());
    if (!socket) return;

    QByteArray buffer = socket->readAll();
    int idx;
    while ((idx = buffer.indexOf('\n')) >= 0) {
        QByteArray line = buffer.left(idx);
        buffer.remove(0, idx + 1);
        if (line.isEmpty()) continue;

        QJsonDocument doc = QJsonDocument::fromJson(line);
        if (!doc.isObject()) continue;
        QJsonObject obj = doc.object();
        QString action = obj.value("action").toString();

        if (action == "refresh") {
            qDebug() << "[TokenRefreshServer] refresh request from client";

            if (!m_authService) {
                sendError(socket, "AuthService not set");
                continue;
            }
            if (m_currentRefreshToken.isEmpty()) {
                sendError(socket, "No refresh token available");
                continue;
            }
            // Вызываем refresh. Результат — через onTokenRefreshed / onSessionExpired.
            m_authService->refreshToken(m_currentRefreshToken);
        }
    }
}

void TokenRefreshServer::onTokenRefreshed(const QString& newAccessToken, const QString& newRefreshToken)
{
    qDebug() << "[TokenRefreshServer] token refreshed, broadcasting to" << m_clients.size() << "clients";
    m_currentRefreshToken = newRefreshToken;
    broadcastToken(newAccessToken);
}

void TokenRefreshServer::onSessionExpired()
{
    qDebug() << "[TokenRefreshServer] session expired, broadcasting error";
    for (auto* c : m_clients) {
        sendError(c, "Session expired");
    }
}

void TokenRefreshServer::broadcastToken(const QString& token)
{
    QJsonObject resp;
    resp["action"] = "token";
    resp["token"] = token;
    QByteArray data = QJsonDocument(resp).toJson(QJsonDocument::Compact) + "\n";
    for (auto* c : m_clients) {
        if (c->state() == QLocalSocket::ConnectedState) {
            c->write(data);
            c->flush();
        }
    }
}

void TokenRefreshServer::sendError(QLocalSocket* socket, const QString& message)
{
    QJsonObject resp;
    resp["action"] = "error";
    resp["message"] = message;
    QByteArray data = QJsonDocument(resp).toJson(QJsonDocument::Compact) + "\n";
    socket->write(data);
    socket->flush();
}
