#include "WidgetBridge.h"
#include "core/DataManager.h"
#include "renderer/JsonUiRenderer.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QQuickItem>
#include <QQuickWindow>
#include <QGuiApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>

WidgetBridge::WidgetBridge(QObject* parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
    qDebug() << "WidgetBridge initialized";
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &WidgetBridge::onHttpReplyFinished);
}

WidgetBridge::~WidgetBridge() {}

void WidgetBridge::setDataManager(DataManager* dataManager)
{
    if (m_dataManager != dataManager) {
        m_dataManager = dataManager;
        emit dataManagerChanged();
        qDebug() << "WidgetBridge: DataManager set";
    }
}

void WidgetBridge::setRenderer(JsonUiRenderer* renderer)
{
    if (m_renderer != renderer) {
        m_renderer = renderer;
        emit rendererChanged();
        qDebug() << "WidgetBridge: Renderer set";
        if (m_renderer && m_dataManager) {
            m_renderer->setDataManager(m_dataManager);
        }
    }
}

void WidgetBridge::loadInterface(const QString& jsonPath)
{
    qDebug() << "WidgetBridge: loadInterface from" << jsonPath;
    
    QFile file(jsonPath);
    if (!file.open(QIODevice::ReadOnly)) {
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
    
    if (parseError.error != QJsonParseError::NoError) {
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
    for (auto window : windows) {
        if (auto quickWindow = qobject_cast<QQuickWindow*>(window)) {
            container = quickWindow->findChild<QQuickItem*>("interfaceContainer");
            if (container) break;
        }
    }
    
    if (!container) {
        emit interfaceError("Cannot find interfaceContainer");
        return;
    }
    
    for (auto child : container->childItems()) {
        child->deleteLater();
    }
    
    if (m_renderer) {
        m_renderer->render(m_currentInterface, container);
        QString title = m_currentInterface["title"].toString("Интерфейс");
        emit interfaceLoaded(title);
    } else {
        emit interfaceError("Renderer not set");
    }
}

void WidgetBridge::requestWidgetData(const QString& widgetId, const QJsonObject& spec)
{
    if (!m_dataManager) {
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

    qDebug() << "WidgetBridge: sendWidgetInput for" << widgetId;
    
    if (input.contains("paramName") && input.contains("value")) {
        m_dataManager->setParameter(input["paramName"].toString(), input["value"].toString());
        emit widgetInputSent(widgetId, true, "Saved");
        refreshAllWidgets();
    } else {
        emit widgetInputSent(widgetId, false, "Missing param");
    }
}

void WidgetBridge::setParameter(const QString& name, const QString& value)
{
    if (m_dataManager) {
        m_dataManager->setParameter(name, value);
    }
}

void WidgetBridge::setParameters(const QJsonObject& params)
{
    for (auto it = params.begin(); it != params.end(); ++it) {
        setParameter(it.key(), it.value().toString());
    }
}

void WidgetBridge::refreshWidget(const QString& widgetId)
{
    Q_UNUSED(widgetId);
    if (m_renderer) {
        m_renderer->refreshAllCharts();
    }
}

void WidgetBridge::refreshAllWidgets()
{
    if (m_renderer) {
        m_renderer->refreshAllCharts();
    }
}

void WidgetBridge::httpRequest(const QString& url, const QString& method, const QString& token, const QString& body, const QString& callbackId)
{
    qDebug() << "WidgetBridge: httpRequest" << method << url;
    
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (!token.isEmpty()) {
        request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());
    }
    
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
    
    m_pendingRequests[reply] = callbackId;
}

void WidgetBridge::onHttpReplyFinished()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    QString callbackId = m_pendingRequests.value(reply, "");
    m_pendingRequests.remove(reply);
    
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray data = reply->readAll();
    
    qDebug() << "WidgetBridge: httpResponse" << callbackId << "status:" << status;
    
    emit httpResponse(callbackId, status, QString::fromUtf8(data));
    
    reply->deleteLater();
}
