#include "GanttApiClient.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QDebug>

GanttApiClient::GanttApiClient(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
{
}

void GanttApiClient::setBaseUrl(const QString& url)
{
    m_baseUrl = url;
}

void GanttApiClient::setToken(const QString& token)
{
    m_token = token;
}

void GanttApiClient::loadMastergraph(const QString& mgId)
{
    if (m_baseUrl.isEmpty() || mgId.isEmpty()) {
        emit errorOccurred("Base URL or MG ID is empty");
        return;
    }

    QUrl url(QString("%1/api/v1/mastergraphs/%2").arg(m_baseUrl, mgId));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (!m_token.isEmpty())
        request.setRawHeader("Authorization", QString("Bearer %1").arg(m_token).toUtf8());

    QNetworkReply* reply = m_manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred(reply->errorString());
            return;
        }
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isNull() || !doc.isObject()) {
            emit errorOccurred("Invalid JSON response");
            return;
        }
        emit mastergraphLoaded(doc.object().toVariantMap());
    });
}

void GanttApiClient::checkAccess(const QString& mgId)
{
    if (m_baseUrl.isEmpty() || mgId.isEmpty()) {
        emit errorOccurred("Base URL or MG ID is empty");
        return;
    }

    QUrl url(QString("%1/api/v1/mastergraphs/%2/access").arg(m_baseUrl, mgId));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (!m_token.isEmpty())
        request.setRawHeader("Authorization", QString("Bearer %1").arg(m_token).toUtf8());

    QNetworkReply* reply = m_manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred(reply->errorString());
            return;
        }
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isNull() || !doc.isObject()) {
            emit errorOccurred("Invalid JSON response");
            return;
        }
        QVariantMap obj = doc.object().toVariantMap();
        emit accessLoaded(
            obj.value("can_edit").toBool(),
            obj.value("can_view").toBool(),
            obj.value("reason").toString()
        );
    });
}

void GanttApiClient::loadHolidays()
{
    if (m_baseUrl.isEmpty()) { emit errorOccurred("Base URL is empty"); return; }
    QUrl url(QString("%1/api/v1/holidays").arg(m_baseUrl));
    QNetworkRequest request(url);
    if (!m_token.isEmpty())
        request.setRawHeader("Authorization", QString("Bearer %1").arg(m_token).toUtf8());
    QNetworkReply* reply = m_manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred(reply->errorString());
            return;
        }
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        if (doc.isNull() || !doc.isObject()) { emit errorOccurred("Invalid JSON (holidays)"); return; }
        emit holidaysLoaded(doc.object().value("holidays").toArray().toVariantList());
    });
}
