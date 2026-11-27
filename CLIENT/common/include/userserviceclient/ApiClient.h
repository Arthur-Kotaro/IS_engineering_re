#ifndef APICLIENT_H
#define APICLIENT_H

#include "UsersServiceClient_global.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QQueue>
#include <functional>
#include <memory>

namespace UsersService
{

struct USERSERVICECLIENT_EXPORT ApiResponse
{
    bool success = false;
    int httpCode = 0;
    QJsonDocument data;
    QString errorString;
    QString errorType;
};

using ApiCallback = std::function<void(const ApiResponse &)>;

class USERSERVICECLIENT_EXPORT ApiClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isOnline READ isOnline NOTIFY onlineStatusChanged)

  public:
    explicit ApiClient(QObject *parent = nullptr);
    virtual ~ApiClient();

    virtual void setServerUrl(const QString &host, int port);
    virtual void setAuthToken(const QString &token);
    virtual void clearAuthToken();
    virtual bool isOnline() const
    {
        return m_isOnline;
    }

    virtual void get(const QString &endpoint, ApiCallback callback);
    virtual void post(const QString &endpoint, const QJsonObject &data, ApiCallback callback);
    virtual void put(const QString &endpoint, const QJsonObject &data, ApiCallback callback);
    virtual void del(const QString &endpoint, ApiCallback callback);

    // Callback для refresh токена. Устанавливается из AuthService.
    // Вызывается при 401 + "Token expired". Возвращает true, если refresh
    // запрошен (запрос будет повторён после tokenRefreshed()).
    using TokenExpiredHandler = std::function<bool()>;
    void setOnTokenExpired(TokenExpiredHandler handler) { m_onTokenExpired = std::move(handler); }

    // Вызывается AuthService после успешного refresh — повторяет очередь.
    void onTokenRefreshed();

    // Вызывается AuthService при неудачном refresh — очищает очередь, эмитит tokenExpired.
    void onTokenRefreshFailed();

  signals:
    void onlineStatusChanged(bool isOnline);
    void tokenExpired();

  private:
    QUrl buildUrl(const QString &endpoint) const;
    void processReply(QNetworkReply *reply, ApiCallback callback);
    void setupSslConfiguration();
    void checkConnectivity();

    // Повтор запроса после refresh
    struct PendingRequest {
        enum Method { Get, Post, Put, Del } method;
        QString endpoint;
        QJsonObject data;
        ApiCallback callback;
    };
    void executeRequest(const PendingRequest &req);
    void enqueueAfterRefresh(const PendingRequest &req);

  protected:
    bool m_isOnline = false;
    QString m_authToken;
    QString m_host;
    int m_port = 8000;
    std::unique_ptr<QNetworkAccessManager> m_nam;

    TokenExpiredHandler m_onTokenExpired;
    bool m_refreshing = false;
    QQueue<PendingRequest> m_pendingAfterRefresh;
};

} // namespace UsersService

#endif // APICLIENT_H
