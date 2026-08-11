#ifndef APICLIENT_H
#define APICLIENT_H

#include "UsersServiceClient_global.h"
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <functional>
#include <memory>

namespace UsersService {

struct USERSERVICECLIENT_EXPORT ApiResponse {
    bool success = false;
    int httpCode = 0;
    QJsonDocument data;
    QString errorString;
    QString errorType;
};

using ApiCallback = std::function<void(const ApiResponse&)>;

class USERSERVICECLIENT_EXPORT ApiClient : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isOnline READ isOnline NOTIFY onlineStatusChanged)

public:
    explicit ApiClient(QObject *parent = nullptr);
    virtual ~ApiClient();
    
    virtual void setServerUrl(const QString& host, int port);
    virtual void setAuthToken(const QString& token);
    virtual void clearAuthToken();
    virtual bool isOnline() const { return m_isOnline; }
    
    virtual void get(const QString& endpoint, ApiCallback callback);
    virtual void post(const QString& endpoint, const QJsonObject& data, ApiCallback callback);
    virtual void put(const QString& endpoint, const QJsonObject& data, ApiCallback callback);
    virtual void del(const QString& endpoint, ApiCallback callback);
    
signals:
    void onlineStatusChanged(bool isOnline);
    void tokenExpired();

private:
    QUrl buildUrl(const QString& endpoint) const;
    void processReply(QNetworkReply* reply, ApiCallback callback);
    void setupSslConfiguration();
    void checkConnectivity();

protected:
    bool m_isOnline = false;
    QString m_authToken;
    QString m_host;
    int m_port = 8000;
    std::unique_ptr<QNetworkAccessManager> m_nam;
};

} // namespace UsersService

#endif // APICLIENT_H
