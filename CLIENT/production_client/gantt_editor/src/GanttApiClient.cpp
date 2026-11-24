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

void GanttApiClient::setUserId(int userId)
{
    m_userId = userId;
}

void GanttApiClient::applyUserIdHeader(QNetworkRequest& request) const
{
    if (m_userId > 0) {
        request.setRawHeader("X-User-ID", QByteArray::number(m_userId));
    }
}

// ---------------- MG Service ----------------

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

// ---------------- BUS Service (X-User-ID) ----------------

void GanttApiClient::createLocalGraph(const QString& taskId, const QString& busUrl, const QVariantMap& payload)
{
    if (busUrl.isEmpty()) { emit errorOccurred("BUS URL is empty"); return; }

    QUrl url(QString("%1/internal/local-graphs/init").arg(busUrl));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    applyUserIdHeader(request);

    QJsonDocument doc = QJsonDocument::fromVariant(payload);
    QNetworkReply* reply = m_manager->post(request, doc.toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, taskId, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred(QString("createLocalGraph (%1): %2").arg(taskId, reply->errorString()));
            return;
        }
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        if (doc.isNull() || !doc.isObject()) { emit errorOccurred("Invalid JSON (createLocalGraph)"); return; }
        emit localGraphCreated(doc.object().toVariantMap());
    });
}

void GanttApiClient::recalcForecast(const QString& taskId, const QString& busUrl, const QVariantMap& payload)
{
    if (busUrl.isEmpty()) { emit errorOccurred("BUS URL is empty"); return; }

    QUrl url(QString("%1/internal/local-graphs/forecast-recalc").arg(busUrl));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    applyUserIdHeader(request);

    QJsonDocument doc = QJsonDocument::fromVariant(payload);
    QNetworkReply* reply = m_manager->post(request, doc.toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, taskId, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred(QString("recalcForecast (%1): %2").arg(taskId, reply->errorString()));
            return;
        }
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        if (doc.isNull() || !doc.isObject()) { emit errorOccurred("Invalid JSON (recalcForecast)"); return; }
        emit forecastRecalculated(doc.object().toVariantMap());
    });
}

void GanttApiClient::loadLocalGraph(const QString& taskId, const QString& busUrl)
{
    if (busUrl.isEmpty() || taskId.isEmpty()) {
        emit errorOccurred("BUS URL or Task ID is empty");
        return;
    }

    QUrl url(QString("%1/internal/local-graphs/by-task/%2").arg(busUrl, taskId));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    applyUserIdHeader(request);

    QNetworkReply* reply = m_manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, busUrl]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred(QString("loadLocalGraph by-task (%1): %2").arg(reply->errorString(), reply->readAll()));
            return;
        }
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        if (doc.isNull() || !doc.isObject()) {
            emit errorOccurred("Invalid JSON (loadLocalGraph)");
            return;
        }

        QVariantMap lgData = doc.object().toVariantMap();
        QString lgId = lgData.value("lg_id").toString();

        QUrl tasksUrl(QString("%1/internal/local-graphs/%2/tasks").arg(busUrl, lgId));
        QNetworkRequest tasksRequest(tasksUrl);
        applyUserIdHeader(tasksRequest);
        QNetworkReply* tasksReply = m_manager->get(tasksRequest);

        connect(tasksReply, &QNetworkReply::finished, this, [this, tasksReply, lgData]() {
            tasksReply->deleteLater();
            QVariantList tasks;
            if (tasksReply->error() == QNetworkReply::NoError) {
                QJsonDocument tasksDoc = QJsonDocument::fromJson(tasksReply->readAll());
                if (tasksDoc.isObject()) {
                    tasks = tasksDoc.object().value("tasks").toArray().toVariantList();
                }
            } else {
                emit errorOccurred(QString("loadLocalGraph tasks: %1").arg(tasksReply->errorString()));
            }
            emit localGraphLoaded(lgData, tasks);
        });
    });
}

// ---------------- BUS: ЛГ-зависимости ----------------

void GanttApiClient::loadLgDependencies(const QString& lgId, const QString& busUrl)
{
    if (busUrl.isEmpty() || lgId.isEmpty()) {
        emit errorOccurred("BUS URL or lgId is empty");
        return;
    }
    QUrl url(QString("%1/internal/local-graphs/%2/dependencies").arg(busUrl, lgId));
    QNetworkRequest request(url);
    applyUserIdHeader(request);
    QNetworkReply* reply = m_manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred(QString("loadLgDependencies: %1").arg(reply->errorString()));
            return;
        }
        QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
        if (doc.isNull() || !doc.isObject()) {
            emit errorOccurred("Invalid JSON (loadLgDependencies)");
            return;
        }
        emit lgDependenciesLoaded(doc.object().value("dependencies").toArray().toVariantList());
    });
}

void GanttApiClient::createLgDependency(const QString& lgId, const QString& busUrl,
                                         const QString& predecessorId, const QString& successorId)
{
    if (busUrl.isEmpty() || lgId.isEmpty()) {
        emit errorOccurred("BUS URL or lgId is empty");
        return;
    }
    QUrl url(QString("%1/internal/local-graphs/%2/dependencies").arg(busUrl, lgId));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    applyUserIdHeader(request);

    QJsonObject payload;
    payload["predecessor_id"] = predecessorId;
    payload["successor_id"] = successorId;
    payload["dep_type"] = "finish_to_start";

    QNetworkReply* reply = m_manager->post(request, QJsonDocument(payload).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        QByteArray body = reply->readAll();
        qDebug() << "[GanttApiClient] createLgDependency FINISHED status=" << status
                 << "error=" << reply->error() << "body=" << body;
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred(QString("createLgDependency: %1 (HTTP %2, body=%3)")
                               .arg(reply->errorString()).arg(status).arg(QString(body)));
            return;
        }
        QJsonDocument doc = QJsonDocument::fromJson(body);
        if (doc.isNull() || !doc.isObject()) {
            emit errorOccurred("Invalid JSON (createLgDependency)");
            return;
        }
        emit lgDependencyCreated(doc.object().toVariantMap());
    });
}

void GanttApiClient::deleteLgDependency(const QString& lgId, const QString& busUrl, const QString& depId)
{
    if (busUrl.isEmpty() || lgId.isEmpty()) {
        emit errorOccurred("BUS URL or lgId is empty");
        return;
    }
    QUrl url(QString("%1/internal/local-graphs/%2/dependencies/%3").arg(busUrl, lgId, depId));
    QNetworkRequest request(url);
    applyUserIdHeader(request);
    QNetworkReply* reply = m_manager->deleteResource(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, depId]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            emit errorOccurred(QString("deleteLgDependency: %1").arg(reply->errorString()));
            return;
        }
        emit lgDependencyDeleted(depId);
    });
}
