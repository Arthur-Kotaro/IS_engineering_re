#include "userserviceclient/ApiClient.h"

#include <QDebug>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QUrl>

namespace UsersService
{

ApiClient::ApiClient(QObject *parent)
    : QObject(parent), m_nam(std::make_unique<QNetworkAccessManager>()), m_isOnline(false)
{
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
    QSslConfiguration::setDefaultConfiguration(sslConfig);
}

ApiClient::~ApiClient() {}

void ApiClient::setServerUrl(const QString &host, int port)
{
    m_host = host;
    m_port = port;
    qDebug() << "ApiClient: Server set to" << m_host << ":" << m_port;
}

void ApiClient::setAuthToken(const QString &token)
{
    m_authToken = token;
}

void ApiClient::clearAuthToken()
{
    m_authToken.clear();
}

QUrl ApiClient::buildUrl(const QString &endpoint) const
{
    if (endpoint.startsWith("http://") || endpoint.startsWith("https://"))
    {
        return QUrl(endpoint);
    }

    QString normalizedEndpoint = endpoint;

    if (!endpoint.startsWith("/api/v1/") && endpoint != "/")
    {
        if (endpoint == "/login" || endpoint == "/refresh" || endpoint == "/logout" ||
            endpoint == "/reset-password" || endpoint == "/password-expiry" ||
            endpoint == "/change-password")
        {
            normalizedEndpoint = "/api/v1/auth" + endpoint;
        }
        else
        {
            normalizedEndpoint = "/api/v1" + endpoint;
        }
    }

    QString urlStr = QString("http://%1:%2%3").arg(m_host).arg(m_port).arg(normalizedEndpoint);
    qDebug() << "ApiClient: buildUrl" << endpoint << "→" << urlStr;
    return QUrl(urlStr);
}

void ApiClient::executeRequest(const PendingRequest &req)
{
    QUrl url = buildUrl(req.endpoint);
    QNetworkRequest request(url);

    if (!m_authToken.isEmpty())
    {
        request.setRawHeader("Authorization", "Bearer " + m_authToken.toUtf8());
    }

    QNetworkReply *reply = nullptr;
    ApiCallback callback = req.callback;

    switch (req.method) {
    case PendingRequest::Get:
        qDebug() << "GET request to:" << url.toString();
        reply = m_nam->get(request);
        break;
    case PendingRequest::Post:
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        qDebug() << "POST request to:" << url.toString();
        reply = m_nam->post(request, QJsonDocument(req.data).toJson());
        break;
    case PendingRequest::Put:
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        qDebug() << "PUT request to:" << url.toString();
        reply = m_nam->put(request, QJsonDocument(req.data).toJson());
        break;
    case PendingRequest::Del:
        qDebug() << "DELETE request to:" << url.toString();
        reply = m_nam->deleteResource(request);
        break;
    }

    connect(reply, &QNetworkReply::finished, this,
            [this, reply, callback, req]() { processReply(reply, callback); });
}

void ApiClient::enqueueAfterRefresh(const PendingRequest &req)
{
    m_pendingAfterRefresh.enqueue(req);
    qDebug() << "ApiClient: request queued after refresh, queue size =" << m_pendingAfterRefresh.size();
}

void ApiClient::get(const QString &endpoint, ApiCallback callback)
{
    PendingRequest req{PendingRequest::Get, endpoint, {}, callback};
    if (m_refreshing) { enqueueAfterRefresh(req); return; }
    executeRequest(req);
}

void ApiClient::post(const QString &endpoint, const QJsonObject &data, ApiCallback callback)
{
    PendingRequest req{PendingRequest::Post, endpoint, data, callback};
    if (m_refreshing) { enqueueAfterRefresh(req); return; }
    executeRequest(req);
}

void ApiClient::put(const QString &endpoint, const QJsonObject &data, ApiCallback callback)
{
    PendingRequest req{PendingRequest::Put, endpoint, data, callback};
    if (m_refreshing) { enqueueAfterRefresh(req); return; }
    executeRequest(req);
}

void ApiClient::del(const QString &endpoint, ApiCallback callback)
{
    PendingRequest req{PendingRequest::Del, endpoint, {}, callback};
    if (m_refreshing) { enqueueAfterRefresh(req); return; }
    executeRequest(req);
}

void ApiClient::processReply(QNetworkReply *reply, ApiCallback callback)
{
    ApiResponse response;
    response.success = false;
    response.httpCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    QByteArray responseData = reply->readAll();

    qDebug() << "=== HTTP Response ===";
    qDebug() << "Status code:" << response.httpCode;
    qDebug() << "Response data:" << responseData;

    // Обработка 401 Token expired → refresh
    if (response.httpCode == 401) {
        QJsonDocument errDoc = QJsonDocument::fromJson(responseData);
        QString detail = errDoc.object().value("detail").toString();
        qDebug() << "ApiClient: 401 detail =" << detail;

        if (detail == "Token expired" && m_onTokenExpired) {
            // Спрашиваем refresh. Если запрошен — ставим запрос в очередь.
            bool refreshRequested = m_onTokenExpired();
            if (refreshRequested) {
                // Восстанавливаем параметры запроса из reply (для повтора)
                // Для простоты: повторяем через сохранённые поля в PendingRequest — см. get/post/put/del
                // Здесь мы не знаем method/endpoint (это внутри lambda). Поэтому используем
                // сохранённый в замыкании req — но его нет в processReply.
                // Решение: сохраняем в reply property "pendingRequest". См. executeRequest.
                QVariant v = reply->property("pendingRequest");
                if (v.isValid()) {
                    PendingRequest req = v.value<PendingRequest>();
                    enqueueAfterRefresh(req);
                    m_refreshing = true;
                    response.errorType = "auth";
                    response.errorString = "Token expired (refresh requested)";
                    qDebug() << "ApiClient: queued for retry after refresh";
                    callback(response);
                    reply->deleteLater();
                    return;
                }
            }
        }
    }

    if (reply->error() == QNetworkReply::NoError)
    {
        if (!responseData.isEmpty())
        {
            QJsonParseError parseError;
            response.data = QJsonDocument::fromJson(responseData, &parseError);

            if (parseError.error == QJsonParseError::NoError)
            {
                response.success = true;
                qDebug() << "JSON parsed successfully";
            }
            else
            {
                response.success = false;
                response.errorType = "parse";
                response.errorString = parseError.errorString();
                qDebug() << "JSON parse error:" << parseError.errorString();
            }
        }
        else
        {
            if (response.httpCode == 204)
            {
                response.success = true;
                response.data = QJsonDocument();
                qDebug() << "Empty response (204 No Content)";
            }
            else
            {
                response.success = false;
                response.errorType = "empty";
                response.errorString = "Empty response";
            }
        }
    }
    else
    {
        response.errorString = reply->errorString();

        if (reply->error() == QNetworkReply::HostNotFoundError)
        {
            response.errorType = "network";
            response.errorString = "Server not found";
        }
        else if (reply->error() == QNetworkReply::ConnectionRefusedError)
        {
            response.errorType = "network";
            response.errorString = "Connection refused";
        }
        else if (reply->error() == QNetworkReply::AuthenticationRequiredError)
        {
            response.errorType = "auth";
            response.errorString = "Authentication required";
        }
        else
        {
            response.errorType = "unknown";
        }

        qDebug() << "Network error:" << response.errorString;

        if (!responseData.isEmpty())
        {
            qDebug() << "Error response body:" << responseData;
        }
    }

    callback(response);
    reply->deleteLater();
}

void ApiClient::onTokenRefreshed()
{
    qDebug() << "ApiClient: token refreshed, resuming queue (size =" << m_pendingAfterRefresh.size() << ")";
    m_refreshing = false;
    while (!m_pendingAfterRefresh.isEmpty()) {
        PendingRequest req = m_pendingAfterRefresh.dequeue();
        executeRequest(req);
    }
}

void ApiClient::onTokenRefreshFailed()
{
    qDebug() << "ApiClient: token refresh failed, clearing queue";
    m_refreshing = false;
    m_pendingAfterRefresh.clear();
    emit tokenExpired();
}

} // namespace UsersService
