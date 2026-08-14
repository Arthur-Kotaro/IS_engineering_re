#include "userserviceclient/ApiClient.h"
#include <QDebug>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSslConfiguration>
#include <QSslSocket>

namespace UsersService {

    ApiClient::ApiClient(QObject* parent)
    : QObject(parent)
    , m_nam(std::make_unique<QNetworkAccessManager>())
    , m_isOnline(false)
    {
        QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
        sslConfig.setPeerVerifyMode(QSslSocket::VerifyNone);
        QSslConfiguration::setDefaultConfiguration(sslConfig);
    }

    ApiClient::~ApiClient() {}

    void ApiClient::setServerUrl(const QString& host, int port) {
        m_host = host;
        m_port = port;
        qDebug() << "ApiClient: Server set to" << m_host << ":" << m_port;
    }

    void ApiClient::setAuthToken(const QString& token) {
        m_authToken = token;
    }

    void ApiClient::clearAuthToken() {
        m_authToken.clear();
    }

    QUrl ApiClient::buildUrl(const QString& endpoint) const {
        if (endpoint.startsWith("http://") || endpoint.startsWith("https://")) {
            return QUrl(endpoint);
        }

        QString normalizedEndpoint = endpoint;
        if (!endpoint.startsWith("/api/v1/") && endpoint != "/") {
            if (endpoint == "/login" || endpoint == "/refresh" || endpoint == "/logout" || 
                endpoint == "/reset-password" || endpoint == "/password-expiry" ||
                endpoint == "/change-password") {
                normalizedEndpoint = "/api/v1/auth" + endpoint;
            }
            else if (endpoint.startsWith("/users/")) {
                normalizedEndpoint = "/api/v1" + endpoint;
            }
            else if (endpoint.startsWith("/navigation/")) {
                normalizedEndpoint = "/api/v1" + endpoint;
            }
            else {
                normalizedEndpoint = "/api/v1" + endpoint;
            }
        }

        QString urlStr = QString("http://%1:%2%3").arg(m_host).arg(m_port).arg(normalizedEndpoint);
        qDebug() << "ApiClient: buildUrl" << endpoint << "→" << urlStr;
        return QUrl(urlStr);
    }

    void ApiClient::get(const QString& endpoint, ApiCallback callback) {
        QUrl url = buildUrl(endpoint);
        QNetworkRequest request(url);

        if (!m_authToken.isEmpty()) {
            request.setRawHeader("Authorization", "Bearer " + m_authToken.toUtf8());
        }

        qDebug() << "GET request to:" << url.toString();

        QNetworkReply* reply = m_nam->get(request);
        connect(reply, &QNetworkReply::finished, this, [this, reply, callback]() {
            processReply(reply, callback);
        });
    }

    void ApiClient::post(const QString& endpoint, const QJsonObject& data, ApiCallback callback) {
        QUrl url = buildUrl(endpoint);
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        if (!m_authToken.isEmpty()) {
            request.setRawHeader("Authorization", "Bearer " + m_authToken.toUtf8());
        }

        qDebug() << "POST request to:" << url.toString();
        qDebug() << "Request data:" << QJsonDocument(data).toJson(QJsonDocument::Compact);

        QNetworkReply* reply = m_nam->post(request, QJsonDocument(data).toJson());
        connect(reply, &QNetworkReply::finished, this, [this, reply, callback]() {
            processReply(reply, callback);
        });
    }

    void ApiClient::put(const QString& endpoint, const QJsonObject& data, ApiCallback callback) {
        QUrl url = buildUrl(endpoint);
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        if (!m_authToken.isEmpty()) {
            request.setRawHeader("Authorization", "Bearer " + m_authToken.toUtf8());
        }

        qDebug() << "PUT request to:" << url.toString();

        QNetworkReply* reply = m_nam->put(request, QJsonDocument(data).toJson());
        connect(reply, &QNetworkReply::finished, this, [this, reply, callback]() {
            processReply(reply, callback);
        });
    }

    void ApiClient::del(const QString& endpoint, ApiCallback callback) {
        QUrl url = buildUrl(endpoint);
        QNetworkRequest request(url);

        if (!m_authToken.isEmpty()) {
            request.setRawHeader("Authorization", "Bearer " + m_authToken.toUtf8());
        }

        qDebug() << "DELETE request to:" << url.toString();

        QNetworkReply* reply = m_nam->deleteResource(request);
        connect(reply, &QNetworkReply::finished, this, [this, reply, callback]() {
            processReply(reply, callback);
        });
    }

    void ApiClient::processReply(QNetworkReply* reply, ApiCallback callback) {
        ApiResponse response;
        response.success = false;
        response.httpCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        QByteArray responseData = reply->readAll();
        
        qDebug() << "=== HTTP Response ===";
        qDebug() << "Status code:" << response.httpCode;
        qDebug() << "Response data:" << responseData;

        if (reply->error() == QNetworkReply::NoError) {
            if (!responseData.isEmpty()) {
                QJsonParseError parseError;
                response.data = QJsonDocument::fromJson(responseData, &parseError);
                
                if (parseError.error == QJsonParseError::NoError) {
                    response.success = true;
                    qDebug() << "JSON parsed successfully";
                } else {
                    response.success = false;
                    response.errorType = "parse";
                    response.errorString = parseError.errorString();
                    qDebug() << "JSON parse error:" << parseError.errorString();
                }
            } else {
                // Пустой ответ — может быть 204 No Content
                if (response.httpCode == 204) {
                    response.success = true;
                    response.data = QJsonDocument();
                    qDebug() << "Empty response (204 No Content)";
                } else {
                    response.success = false;
                    response.errorType = "empty";
                    response.errorString = "Empty response";
                }
            }
        } else {
            response.errorString = reply->errorString();

            if (reply->error() == QNetworkReply::HostNotFoundError) {
                response.errorType = "network";
                response.errorString = "Server not found";
            } else if (reply->error() == QNetworkReply::ConnectionRefusedError) {
                response.errorType = "network";
                response.errorString = "Connection refused";
            } else if (reply->error() == QNetworkReply::AuthenticationRequiredError) {
                response.errorType = "auth";
                response.errorString = "Authentication required";
            } else {
                response.errorType = "unknown";
            }

            qDebug() << "Network error:" << response.errorString;
            
            // Пытаемся получить тело ошибки
            if (!responseData.isEmpty()) {
                qDebug() << "Error response body:" << responseData;
            }
        }

        callback(response);
        reply->deleteLater();
    }

} // namespace UsersService
