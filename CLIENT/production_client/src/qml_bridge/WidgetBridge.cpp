#include "WidgetBridge.h"

#include "core/DataManager.h"
#include "renderer/JsonUiRenderer.h"

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QGuiApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QQuickItem>
#include <QQuickWindow>
#include <QUrl>

WidgetBridge::WidgetBridge(QObject *parent)
    : QObject(parent), m_networkManager(new QNetworkAccessManager(this))
{
    qDebug() << "WidgetBridge initialized";
    connect(m_networkManager, &QNetworkAccessManager::finished, this,
            &WidgetBridge::onNetworkReplyFinished);
}

WidgetBridge::~WidgetBridge() {}

void WidgetBridge::setDataManager(DataManager *dataManager)
{
    if (m_dataManager != dataManager)
    {
        m_dataManager = dataManager;
        m_currentToken.clear();
        if (m_dataManager)
        {
            m_dataManager->setParameter("__access_token__", m_currentToken);
        }
    }
}

void WidgetBridge::setRenderer(JsonUiRenderer *renderer)
{
    if (m_renderer != renderer)
    {
        m_renderer = renderer;
        if (m_renderer && m_dataManager)
        {
            m_renderer->setDataManager(m_dataManager);
        }
    }
}

void WidgetBridge::loadInterface(const QString &jsonPath)
{
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

void WidgetBridge::loadInterfaceFromJson(const QString &jsonString)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError)
    {
        emit interfaceError("JSON parse error: " + parseError.errorString());
        return;
    }

    if (!doc.isObject())
    {
        emit interfaceError("JSON must be object");
        return;
    }

    m_currentInterface = doc.object();

    QQuickItem *container = nullptr;
    auto windows = QGuiApplication::topLevelWindows();
    for (auto window : windows)
    {
        if (auto quickWindow = qobject_cast<QQuickWindow *>(window))
        {
            container = quickWindow->findChild<QQuickItem *>("interfaceContainer");
            if (container)
            {
                break;
            }
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

void WidgetBridge::requestWidgetData(const QString &widgetId, const QJsonObject &spec)
{
    if (!m_dataManager)
    {
        qWarning() << "WidgetBridge: No DataManager";
        return;
    }
    m_dataManager->requestData(widgetId, spec);
}

void WidgetBridge::sendWidgetInput(const QString &widgetId, const QJsonObject &input)
{
    if (!m_dataManager)
    {
        emit widgetInputSent(widgetId, false, "No DataManager");
        return;
    }

    if (input.contains("paramName"))
    {
        QString name = input["paramName"].toString();
        QJsonValue val = input["value"];
        QString strVal;

        if (val.isArray())
        {
            QJsonDocument doc(val.toArray());
            strVal = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
        }
        else
        {
            strVal = val.toVariant().toString();
        }

        m_dataManager->setParameter(name, strVal);
        emit widgetInputSent(widgetId, true, "Saved");
    }
    else
    {
        emit widgetInputSent(widgetId, false, "Missing param");
    }
}

void WidgetBridge::setParameter(const QString &name, const QString &value)
{
    if (m_dataManager)
    {
        m_dataManager->setParameter(name, value);
    }
}

void WidgetBridge::setParameters(const QJsonObject &params)
{
    for (auto it = params.begin(); it != params.end(); ++it)
    {
        setParameter(it.key(), it.value().toString());
    }
}

void WidgetBridge::refreshWidget(const QString &widgetId)
{
    Q_UNUSED(widgetId);
    if (m_renderer)
    {
        m_renderer->refreshAllCharts();
    }
}

void WidgetBridge::refreshAllWidgets()
{
    if (m_renderer)
    {
        m_renderer->refreshAllCharts();
    }
}

void WidgetBridge::httpRequest(const QString &url, const QString &method, const QString &token,
                               const QString &body, const QString &callbackId)
{
    qDebug() << "WidgetBridge: httpRequest" << method << url << "callbackId:" << callbackId;

    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (!token.isEmpty())
    {
        request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());
    }

    QNetworkReply *reply = nullptr;
    if (method == "GET")
    {
        reply = m_networkManager->get(request);
    }
    else if (method == "POST")
    {
        reply = m_networkManager->post(request, body.toUtf8());
    }
    else if (method == "PUT")
    {
        reply = m_networkManager->put(request, body.toUtf8());
    }
    else if (method == "DELETE")
    {
        reply = m_networkManager->deleteResource(request);
    }
    else
    {
        emit httpResponse(callbackId, 0, "Unknown method");
        return;
    }

    if (reply)
    {
        m_pendingRequests[reply] = callbackId;
    }
    else
    {
        emit httpResponse(callbackId, 0, "Failed to send request");
    }
}

void WidgetBridge::onNetworkReplyFinished(QNetworkReply *reply)
{
    if (!reply)
    {
        return;
    }

    bool isWorkflow = m_workflowRequests.value(reply, false);
    m_workflowRequests.remove(reply);

    QString callbackId = m_pendingRequests.value(reply, "");
    m_pendingRequests.remove(reply);

    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray data = reply->readAll();

    qDebug() << "WidgetBridge: onNetworkReplyFinished - callbackId:" << callbackId
             << "status:" << status << "isWorkflow:" << isWorkflow;

    if (isWorkflow)
    {
        if (status == 200)
        {
            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
            if (parseError.error == QJsonParseError::NoError && doc.isObject())
            {
                emit workflowEventCompleted(doc.object());
            }
            else
            {
                qWarning() << "WidgetBridge: workflow response parse error";
                emit httpResponse(callbackId, status, QString::fromUtf8(data));
            }
        }
        else
        {
            emit httpResponse(callbackId, status, QString::fromUtf8(data));
        }
    }
    else
    {
        emit httpResponse(callbackId, status, QString::fromUtf8(data));
    }

    reply->deleteLater();
}

void WidgetBridge::renderPage(const QJsonObject &uiData, QQuickItem *container)
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

// Нормализация: строки вида "[...]" -> массивы, элементы-числа -> int.
// Нужно для MultiSelectField (role_ids), который приходит как строка "[\"51\"]".
QJsonObject WidgetBridge::normalizeArrayValues(const QJsonObject &body)
{
    QJsonObject normalizedBody;
    for (auto it = body.begin(); it != body.end(); ++it)
    {
        QString key = it.key();
        QJsonValue val = it.value();
        if (val.isString())
        {
            QString s = val.toString().trimmed();
            if (s.startsWith("[") && s.endsWith("]"))
            {
                QJsonParseError err;
                QJsonDocument doc = QJsonDocument::fromJson(s.toUtf8(), &err);
                if (err.error == QJsonParseError::NoError && doc.isArray())
                {
                    QJsonArray arr = doc.array();
                    QJsonArray normalized;
                    for (const QJsonValue &v : arr)
                    {
                        if (v.isString())
                        {
                            bool ok = false;
                            int n = v.toString().toInt(&ok);
                            if (ok)
                            {
                                normalized.append(n);
                            }
                            else
                            {
                                normalized.append(v);
                            }
                        }
                        else
                        {
                            normalized.append(v);
                        }
                    }
                    normalizedBody[key] = normalized;
                    qDebug() << "normalizeArrayValues: normalized array for" << key
                             << "size=" << normalized.size();
                    continue;
                }
            }
        }
        normalizedBody[key] = val;
    }
    return normalizedBody;
}

void WidgetBridge::sendWorkflowEvent(const QString &widgetId, const QString &eventType,
                                     const QJsonObject &data)
{
    qDebug() << "WidgetBridge: sendWorkflowEvent widget=" << widgetId << "type=" << eventType
             << "sessionId.length=" << m_currentSessionId.length();

    if (m_currentSessionId.isEmpty())
    {
        qWarning() << "WidgetBridge: no current session id - cannot send workflow event";
        emit widgetInputSent(widgetId, false, "No session id");
        return;
    }

    QJsonObject normalizedData = normalizeArrayValues(data);

    QJsonObject payload;
    payload["session_id"] = m_currentSessionId;
    payload["widget_id"] = widgetId;
    payload["event_type"] = eventType;
    payload["data"] = normalizedData;

    QNetworkRequest request(QUrl("http://localhost:8080/workflow/event"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (!m_currentToken.isEmpty())
    {
        request.setRawHeader("Authorization", ("Bearer " + m_currentToken).toUtf8());
    }

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(payload).toJson());
    m_pendingRequests[reply] =
        "wf_" + widgetId + "_" + QString::number(QDateTime::currentMSecsSinceEpoch());
    m_workflowRequests[reply] = true;
}

QString WidgetBridge::getParameter(const QString &name) const
{
    if (!m_dataManager)
    {
        return QString();
    }
    return m_dataManager->getParameter(name);
}

void WidgetBridge::submitForm(const QString &endpoint, const QString &method,
                              const QJsonObject &body, const QString &callbackId)
{
    if (!m_dataManager)
    {
        emit httpResponse(callbackId, 0, "No DataManager");
        return;
    }

    QString url = endpoint;
    if (!url.startsWith("http"))
    {
        url = "http://localhost:8080" + url;
    }

    QNetworkRequest request{QUrl(url)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QString token = m_dataManager->getParameter("__access_token__");
    if (!token.isEmpty())
    {
        request.setRawHeader("Authorization", ("Bearer " + token).toUtf8());
    }

    QJsonObject normalizedBody = normalizeArrayValues(body);
    QByteArray payload = QJsonDocument(normalizedBody).toJson();
    qDebug() << "submitForm: payload =" << QString::fromUtf8(payload);

    QNetworkReply *reply = nullptr;

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

void WidgetBridge::setAccessToken(const QString &token)
{
    m_currentToken = token;
    if (m_dataManager)
    {
        m_dataManager->setParameter("__access_token__", token);
    }
}

void WidgetBridge::setCurrentSessionId(const QString &sessionId)
{
    m_currentSessionId = sessionId;
    qDebug() << "WidgetBridge: current session id set, length =" << sessionId.length();
}

void WidgetBridge::clearParameters()
{
    if (m_dataManager)
    {
        m_dataManager->clearParameters();
        qDebug() << "WidgetBridge: parameters cleared";
    }
}
