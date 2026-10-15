#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QCache>
#include <QJsonDocument>
#include <QMap>
#include <QNetworkAccessManager>
#include <QObject>
#include <QTimer>
#include <functional>

class DataManager : public QObject
{
    Q_OBJECT

  public:
    explicit DataManager(QObject *parent = nullptr);
    ~DataManager();

    void requestData(const QString &widgetId, const QJsonObject &dataSpec);

    void cancelRequest(const QString &widgetId);
    void cancelAllRequests();

    void setCacheTTL(int seconds);
    void clearCache();

    void setParameter(const QString &name, const QString &value);
    QString getParameter(const QString &name) const;
    void clearParameters();
    void removeParameter(const QString &name);

  signals:
    void dataReady(const QString &widgetId, const QJsonDocument &data);
    void dataProgress(const QString &widgetId, int percent);
    void dataError(const QString &widgetId, const QString &error, const QString &endpoint,
                   int httpCode);

  private:
    struct PendingRequest
    {
        QString widgetId;
        QString url;
        int attempt = 0;
        int maxAttempts = 3;
        QTimer *timeoutTimer = nullptr;
        QNetworkReply *reply = nullptr;
        std::function<void(const QJsonDocument &)> callback;
    };

    struct CacheEntry
    {
        QByteArray data;
        QDateTime timestamp;
    };

    void processInlineData(const QString &widgetId, const QJsonObject &dataSpec);
    void processEndpointData(const QString &widgetId, const QJsonObject &dataSpec);

    QString buildUrl(const QJsonObject &dataSpec);
    QString substituteParameters(const QString &templateStr);
    void sendRequest(PendingRequest *request);
    void handleReply(QNetworkReply *reply);
    void retryRequest(PendingRequest *request);
    void finishRequest(PendingRequest *request, bool success,
                       const QJsonDocument &data = QJsonDocument());

    QNetworkAccessManager *m_network;
    QMap<QString, PendingRequest *> m_pendingRequests;
    QMap<QString, CacheEntry> m_cache;
    QMap<QString, QString> m_parameters;
    int m_cacheTTL = 60;
    int m_timeout = 30000;
};

#endif // DATAMANAGER_H

Q_DECLARE_METATYPE(DataManager *)
