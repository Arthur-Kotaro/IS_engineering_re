#include "DataManager.h"
#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QNetworkReply>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>

DataManager::DataManager(QObject* parent)
    : QObject(parent)
    , m_network(new QNetworkAccessManager(this))
{
    connect(m_network, &QNetworkAccessManager::finished,
            this, &DataManager::handleReply);
    qDebug() << "DataManager initialized";
}

DataManager::~DataManager()
{
    cancelAllRequests();
}

void DataManager::requestData(const QString& widgetId, const QJsonObject& dataSpec)
{
    QString source = dataSpec["source"].toString("inline");
    
    if (source == "inline") {
        processInlineData(widgetId, dataSpec);
    } else if (source == "endpoint") {
        processEndpointData(widgetId, dataSpec);
    } else {
        emit dataError(widgetId, "Unknown data source: " + source, "", 0);
    }
}

void DataManager::processInlineData(const QString& widgetId, const QJsonObject& dataSpec)
{
    QJsonDocument doc(dataSpec);
    emit dataReady(widgetId, doc);
    qDebug() << "DataManager: inline data for" << widgetId;
}

void DataManager::processEndpointData(const QString& widgetId, const QJsonObject& dataSpec)
{
    // Проверяем кеш
    QString url = buildUrl(dataSpec);
    
    if (m_cache.contains(url)) {
        CacheEntry entry = m_cache[url];
        if (entry.timestamp.secsTo(QDateTime::currentDateTime()) < m_cacheTTL) {
            QJsonDocument doc = QJsonDocument::fromJson(entry.data);
            emit dataReady(widgetId, doc);
            qDebug() << "DataManager: cache hit for" << widgetId << "→" << url;
            return;
        } else {
            m_cache.remove(url);
        }
    }

    // Создаём запрос
    PendingRequest* request = new PendingRequest();
    request->widgetId = widgetId;
    request->url = url;
    request->attempt = 0;
    request->maxAttempts = dataSpec["max_attempts"].toInt(3);
    
    // Таймаут
    request->timeoutTimer = new QTimer(this);
    request->timeoutTimer->setSingleShot(true);
    request->timeoutTimer->setInterval(m_timeout);
    connect(request->timeoutTimer, &QTimer::timeout, this, [this, request]() {
        if (request->reply) {
            request->reply->abort();
            request->reply->deleteLater();
            request->reply = nullptr;
        }
        retryRequest(request);
    });

    m_pendingRequests[widgetId] = request;
    sendRequest(request);
}

QString DataManager::buildUrl(const QJsonObject& dataSpec)
{
    QString urlTemplate = dataSpec["url"].toString();
    QString url = substituteParameters(urlTemplate);
    
    // Добавляем параметры
    if (dataSpec.contains("params") && dataSpec["params"].isObject()) {
        QJsonObject params = dataSpec["params"].toObject();
        QUrlQuery query;
        for (const QString& key : params.keys()) {
            QString value = substituteParameters(params[key].toString());
            query.addQueryItem(key, value);
        }
        if (!query.isEmpty()) {
            url += "?" + query.toString();
        }
    }
    
    return url;
}

QString DataManager::substituteParameters(const QString& templateStr)
{
    QString result = templateStr;
    for (const QString& key : m_parameters.keys()) {
        QString placeholder = "{" + key + "}";
        result.replace(placeholder, m_parameters[key]);
    }
    return result;
}

void DataManager::sendRequest(PendingRequest* request)
{
    request->attempt++;
    
    QUrl url(request->url);
    if (!url.isValid()) {
        emit dataError(request->widgetId, "Invalid URL: " + request->url, request->url, 0);
        finishRequest(request, false);
        return;
    }

    qDebug() << "DataManager: request" << request->attempt << "/" << request->maxAttempts 
             << "for" << request->widgetId << "→" << url.toString();

    QNetworkRequest networkRequest(url);
    networkRequest.setHeader(QNetworkRequest::UserAgentHeader, "EngineeringClient/1.0");
    
    request->reply = m_network->get(networkRequest);
    request->timeoutTimer->start();
}

void DataManager::handleReply(QNetworkReply* reply)
{
    QString widgetId;
    PendingRequest* request = nullptr;
    
    // Находим запрос по reply
    for (auto it = m_pendingRequests.begin(); it != m_pendingRequests.end(); ++it) {
        if (it.value()->reply == reply) {
            request = it.value();
            widgetId = it.key();
            break;
        }
    }
    
    if (!request) {
        reply->deleteLater();
        return;
    }

    request->timeoutTimer->stop();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "DataManager: network error for" << widgetId << ":" << reply->errorString();
        retryRequest(request);
        reply->deleteLater();
        return;
    }

    int httpCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray data = reply->readAll();
    reply->deleteLater();

    if (httpCode >= 200 && httpCode < 300) {
        // Успех
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
        
        if (parseError.error != QJsonParseError::NoError) {
            emit dataError(widgetId, "JSON parse error: " + parseError.errorString(), request->url, httpCode);
            finishRequest(request, false);
            return;
        }

        // Сохраняем в кеш
        CacheEntry entry;
        entry.data = data;
        entry.timestamp = QDateTime::currentDateTime();
        m_cache[request->url] = entry;

        emit dataReady(widgetId, doc);
        finishRequest(request, true);
    } else {
        // HTTP ошибка
        emit dataError(widgetId, "HTTP error: " + QString::number(httpCode), request->url, httpCode);
        retryRequest(request);
    }
}

void DataManager::retryRequest(PendingRequest* request)
{
    if (request->attempt < request->maxAttempts) {
        // Экспоненциальная задержка: 1с, 3с, 7с
        int delay = (1 << (request->attempt - 1)) * 1000 + 500;
        qDebug() << "DataManager: retry" << request->attempt + 1 << "for" << request->widgetId 
                 << "in" << delay << "ms";
        
        QTimer::singleShot(delay, this, [this, request]() {
            if (m_pendingRequests.contains(request->widgetId)) {
                sendRequest(request);
            }
        });
    } else {
        // Все попытки исчерпаны
        emit dataError(request->widgetId, "Max attempts exceeded (" + QString::number(request->maxAttempts) + ")", request->url, 0);
        finishRequest(request, false);
    }
}

void DataManager::finishRequest(PendingRequest* request, bool success, const QJsonDocument& data)
{
    Q_UNUSED(data);
    if (request->reply) {
        request->reply->deleteLater();
        request->reply = nullptr;
    }
    if (request->timeoutTimer) {
        request->timeoutTimer->stop();
        request->timeoutTimer->deleteLater();
    }
    m_pendingRequests.remove(request->widgetId);
    delete request;
}

void DataManager::cancelRequest(const QString& widgetId)
{
    if (m_pendingRequests.contains(widgetId)) {
        PendingRequest* request = m_pendingRequests[widgetId];
        if (request->reply) {
            request->reply->abort();
            request->reply->deleteLater();
        }
        finishRequest(request, false);
    }
}

void DataManager::cancelAllRequests()
{
    for (const QString& widgetId : m_pendingRequests.keys()) {
        cancelRequest(widgetId);
    }
}

void DataManager::setCacheTTL(int seconds)
{
    m_cacheTTL = seconds;
}

void DataManager::clearCache()
{
    m_cache.clear();
}

void DataManager::setParameter(const QString& name, const QString& value)
{
    m_parameters[name] = value;
    qDebug() << "DataManager: parameter set" << name << "=" << value;
}

QString DataManager::getParameter(const QString& name) const
{
    return m_parameters.value(name);
}
