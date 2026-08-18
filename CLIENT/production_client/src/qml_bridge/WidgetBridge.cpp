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

WidgetBridge::WidgetBridge(QObject* parent)
    : QObject(parent)
{
    qDebug() << "WidgetBridge initialized";
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
        emit interfaceError("JSON must be an object");
        return;
    }
    
    m_currentInterface = doc.object();
    
    // Ищем контейнер в QML
    QQuickItem* container = nullptr;
    auto windows = QGuiApplication::topLevelWindows();
    for (auto window : windows) {
        if (auto quickWindow = qobject_cast<QQuickWindow*>(window)) {
            container = quickWindow->findChild<QQuickItem*>("interfaceContainer");
            if (container) break;
        }
    }
    
    if (!container) {
        emit interfaceError("Cannot find interfaceContainer in QML");
        return;
    }
    
    // Очищаем старый контент
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
        qWarning() << "WidgetBridge: No DataManager set";
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
        emit widgetInputSent(widgetId, true, "Parameter saved");
        refreshAllWidgets();
    } else {
        emit widgetInputSent(widgetId, false, "Missing paramName or value");
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
