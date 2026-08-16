#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QCache>
#include <QMap>
#include <QTimer>
#include <QJsonDocument>
#include <functional>

class DataManager : public QObject
{
    Q_OBJECT

public:
    explicit DataManager(QObject* parent = nullptr);
    ~DataManager();

    // Основной метод запроса данных
    void requestData(const QString& widgetId, const QJsonObject& dataSpec);
    
    // Отмена запросов
    void cancelRequest(const QString& widgetId);
    void cancelAllRequests();

    // Управление кешем
    void setCacheTTL(int seconds);
    void clearCache();

    // Подстановка параметров (для фильтров)
    void setParameter(const QString& name, const QString& value);
    QString getParameter(const QString& name) const;

signals:
    void dataReady(const QString& widgetId, const QJsonDocument& data);
    void dataProgress(const QString& widgetId, int percent);
    void dataError(const QString& widgetId, const QString& error, const QString& endpoint, int httpCode);

private:
    struct PendingRequest {
        QString widgetId;
        QString url;
        int attempt = 0;
        int maxAttempts = 3;
        QTimer* timeoutTimer = nullptr;
        QNetworkReply* reply = nullptr;
        std::function<void(const QJsonDocument&)> callback;
    };

    struct CacheEntry {
        QByteArray data;
        QDateTime timestamp;
    };

    void processInlineData(const QString& widgetId, const QJsonObject& dataSpec);
    void processEndpointData(const QString& widgetId, const QJsonObject& dataSpec);
    
    QString buildUrl(const QJsonObject& dataSpec);
    QString substituteParameters(const QString& templateStr);
    void sendRequest(PendingRequest* request);
    void handleReply(QNetworkReply* reply);
    void retryRequest(PendingRequest* request);
    void finishRequest(PendingRequest* request, bool success, const QJsonDocument& data = QJsonDocument());

    QNetworkAccessManager* m_network;
    QMap<QString, PendingRequest*> m_pendingRequests;
    QMap<QString, CacheEntry> m_cache;
    QMap<QString, QString> m_parameters;
    int m_cacheTTL = 60; // секунд
    int m_timeout = 30000; // 30 секунд
};

#endif // DATAMANAGER_H
