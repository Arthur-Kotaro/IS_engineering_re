#include "WidgetBridge.h"
#include "core/DataManager.h"
#include "renderer/JsonUiRenderer.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QDebug>
#include <QQuickItem>
#include <QQuickWindow>
#include <QGuiApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>

WidgetBridge::WidgetBridge(QObject* parent): QObject(parent), m_networkManager(new QNetworkAccessManager(this))
{
    qDebug() << "WidgetBridge initialized";
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &WidgetBridge::onNetworkReplyFinished);
}

WidgetBridge::~WidgetBridge() {}

void WidgetBridge::setDataManager(DataManager* dataManager)
{
    if (m_dataManager != dataManager)
    {
        m_dataManager = dataManager;
        qDebug() << "WidgetBridge: DataManager set";
        m_currentToken.clear();
        if (m_dataManager) m_dataManager->setParameter("__access_token__", m_currentToken);
    }
}

void WidgetBridge::setRenderer(JsonUiRenderer* renderer)
{
    if (m_renderer != renderer)
    {
        m_renderer = renderer;
        qDebug() << "WidgetBridge: Renderer set";
        if (m_renderer && m_dataManager) m_renderer->setDataManager(m_dataManager);
    }
}

void WidgetBridge::loadInterface(const QString& jsonPath)
{
    qDebug() << "WidgetBridge: loadInterface from" << jsonPath;

    QFile file(jsonPath);
    if (!file.open(QIODevice::ReadOnly))
    {
        emit interfaceError("Cannot open file: " + jsonPath);
        return;
    }

    QByteArray data = file.readAll();
    file.close();
    loadInterfaceFromJson(QString::fromUtf8(data));
}

void WidgetBridge::loadInterfaceFromJson(const QString& jsonString)
{
    qDebug() << "WidgetBridge: loadInterfaceFromJson";

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError)
    {
        emit interfaceError("JSON parse error: " + parseError.errorString());
        return;
    }

    if (!doc.isObject()) {
        emit interfaceError("JSON must be object");
        return;
    }

    m_currentInterface = doc.object();

    QQuickItem* container = nullptr;
    auto windows = QGuiApplication::topLevelWindows();
    for (auto window : windows)
    {
        if (auto quickWindow = qobject_cast<QQuickWindow*>(window))
        {
            container = quickWindow->findChild<QQuickItem*>("interfaceContainer");
            if (container) break;
        }
    }

    if (!container)
    {
        emit interfaceError("Cannot find interfaceContainer");
        return;
    }

    for (auto child : container->childItems())
    {
        child->deleteLater();
    }

    if (m_renderer)
    {
        m_renderer->render(m_currentInterface, container);
        QString title = m_currentInterface["title"].toString("Интерфейс");
        emit interfaceLoaded(title);
    }
    else
    {
        emit interfaceError("Renderer not set");
    }
}

void WidgetBridge::requestWidgetData(const QString& widgetId, const QJsonObject& spec)
{
    if (!m_dataManager)
    {
        qWarning() << "WidgetBridge: No DataManager";
        return;
    }
    qDebug() << "WidgetBridge: requestWidgetData for" << widgetId;
    m_dataManager->requestData(widgetId, spec);
}

void WidgetBridge::sendWidgetInput(const QString& widgetId, const QJsonObject& input)
{
    if (!m_dataManager) {
        emit widgetInputSent(widgetId, false, "No DataManager");
        return;
    }

    if (input.contains("paramName")) {
        QString name = input["paramName"].toString();
        QJsonValue val = input["value"];
        QString strVal;

        if (val.isArray()) {
            QJsonDocument doc(val.toArray());
            strVal = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
        } else {
            strVal = val.toVariant().toString();
        }

        m_dataManager->setParameter(name, strVal);
        emit widgetInputSent(widgetId, true, "Saved");
    } else {
        emit widgetInputSent(widgetId, false, "Missing param");
    }
}

void WidgetBridge::setParameter(const QString& name, const QString& value)
{
    if (m_dataManager) m_dataManager->setParameter(name, value);
}

void WidgetBridge::setParameters(const QJsonObject& params)
{
    for (auto it = params.begin(); it != params.end(); ++it)
    {
        setParameter(it.key(), it.value().toString());
    }
}

void WidgetBridge::refreshWidget(const QString& widgetId)
{
    Q_UNUSED(widgetId);
    if (m_renderer) m_renderer->refreshAllCharts();
}

void WidgetBridge::refreshAllWidgets()
{
    if (m_renderer) m_renderer->refreshAllCharts();
}

void WidgetBridge::httpRequest(const QString& url, const QString& method, const QString& token, const QString& body, const QString& callbackId)
{
    qDebug() << "WidgetBridge: httpRequest" << method << url << "callbackId:" << callbackId;

    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (!token.isEmpty()) request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());

    QNetworkReply* reply = nullptr;
    if (method == "GET") {
        reply = m_networkManager->get(request);
    } else if (method == "POST") {
        reply = m_networkManager->post(request, body.toUtf8());
    } else if (method == "PUT") {
        reply = m_networkManager->put(request, body.toUtf8());
    } else if (method == "DELETE") {
        reply = m_networkManager->deleteResource(request);
    } else {
        qWarning() << "Unknown HTTP method:" << method;
        emit httpResponse(callbackId, 0, "Unknown method");
        return;
    }

    if (reply)
    {
        m_pendingRequests[reply] = callbackId;
        qDebug() << "WidgetBridge: request sent, callbackId stored:" << callbackId << "reply:" << reply;
    }
    else
    {
        qDebug() << "WidgetBridge: ERROR - reply is null!";
        emit httpResponse(callbackId, 0, "Failed to send request");
    }
}

void WidgetBridge::onNetworkReplyFinished(QNetworkReply* reply)
{
    if (!reply)
    {
        qDebug() << "WidgetBridge: onNetworkReplyFinished - reply is null!";
        return;
    }

    QString callbackId = m_pendingRequests.value(reply, "");
    m_pendingRequests.remove(reply);

    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray data = reply->readAll();

    qDebug() << "WidgetBridge: onNetworkReplyFinished - callbackId:" << callbackId << "status:" << status;
    qDebug() << "WidgetBridge: response data size:" << data.size();
    if (data.size() > 0) {
        qDebug() << "WidgetBridge: response data preview:" << QString(data.left(200));
    }

    emit httpResponse(callbackId, status, QString::fromUtf8(data));

    reply->deleteLater();
}

void WidgetBridge::renderPage(const QJsonObject& uiData, QQuickItem* container)
{
    if (m_renderer && container)
    {
        m_renderer->render(uiData, container);
    }
    else
    {
        qWarning() << "WidgetBridge: Cannot render - renderer or container is null";
    }
}

void WidgetBridge::sendWorkflowEvent(const QString& sessionId, const QString& widgetId,
                                     const QString& eventType, const QJsonObject& data, const QString& accessToken)
{
    QJsonObject payload;
    payload["session_id"] = sessionId;
    payload["widget_id"] = widgetId;
    payload["event_type"] = eventType;
    payload["data"] = data;

    QNetworkRequest request(QUrl("http://localhost:8080/workflow/event"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (!accessToken.isEmpty())  request.setRawHeader("Authorization", ("Bearer " + accessToken).toUtf8());

    QString cbId = "wf_" + widgetId + "_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    QNetworkReply* reply = m_networkManager->post(request, QJsonDocument(payload).toJson());
    m_pendingRequests[reply] = cbId;
}


QString WidgetBridge::getParameter(const QString& name) const
{
    if (!m_dataManager) return QString();
    return m_dataManager->getParameter(name);
}

void WidgetBridge::submitForm(const QString& endpoint, const QString& method, const QJsonObject& body, const QString& callbackId)
{
    if (!m_dataManager)
    {
        emit httpResponse(callbackId, 0, "No DataManager");
        return;
    }

    QString url = endpoint;
    if (!url.startsWith("http")) url = "http://localhost:8080" + url;

    QNetworkRequest request{QUrl(url)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QString token = m_dataManager->getParameter("__access_token__");
    if (!token.isEmpty()) request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());

    QNetworkReply* reply = nullptr;
    QByteArray payload = QJsonDocument(body).toJson();

    if (method == "POST")
    {
        reply = m_networkManager->post(request, payload);
    }
    else if (method == "PUT")
    {
        reply = m_networkManager->put(request, payload);
    }
    else
    {
        reply = m_networkManager->post(request, payload);
    }

    m_pendingRequests[reply] = callbackId;
}

void WidgetBridge::setAccessToken(const QString& token)
{
    m_currentToken = token;
    if (m_dataManager) {
        m_dataManager->setParameter("__access_token__", token);
    }
    qDebug() << "WidgetBridge: access token set, length =" << token.length();
}
