#include "JsonUiRenderer.h"
#include "QmlObjectFactory.h"
#include "core/DataManager.h"
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQuickItem>
#include <QDebug>
#include <QJsonArray>
#include <QJsonValue>

JsonUiRenderer::JsonUiRenderer(QQmlEngine* engine, QObject* parent)
    : QObject(parent)
    , m_engine(engine)
    , m_factory(new QmlObjectFactory(engine, this))
    , m_dataManager(new DataManager(this))
{
    qDebug() << "JsonUiRenderer initialized";

    // Подключаем сигналы DataManager
    connect(m_dataManager, &DataManager::dataReady,
            this, &JsonUiRenderer::onDataReady);
    connect(m_dataManager, &DataManager::dataError,
            this, &JsonUiRenderer::onDataError);
    connect(m_dataManager, &DataManager::dataProgress,
            this, &JsonUiRenderer::onDataProgress);
}

JsonUiRenderer::~JsonUiRenderer()
{
    qDebug() << "JsonUiRenderer destroyed";
}

void JsonUiRenderer::render(const QJsonObject& root, QQuickItem* container)
{
    emit renderStarted();

    if (!container) {
        qWarning() << "Container is null";
        emit renderFinished();
        return;
    }

    qDebug() << "Rendering JSON root object";

    QJsonArray widgets = root["widgets"].toArray();
    if (widgets.isEmpty()) {
        qWarning() << "No widgets found in JSON";
        emit renderFinished();
        return;
    }

    // Создаём корневой ColumnLayout внутри контейнера
    QString layoutQml = 
        "import QtQuick 6.0\n"
        "import QtQuick.Controls 6.0\n"
        "import QtQuick.Layouts 6.0\n"
        "ColumnLayout {\n"
        "    anchors.fill: parent\n"
        "    spacing: 12\n"
        "}\n";

    QQmlComponent layoutComponent(m_engine);
    layoutComponent.setData(layoutQml.toUtf8(), QUrl());

    if (layoutComponent.isError()) {
        qWarning() << "Failed to create ColumnLayout:" << layoutComponent.errorString();
        emit renderFinished();
        return;
    }

    QObject* layoutObj = layoutComponent.create();
    if (!layoutObj) {
        qWarning() << "Failed to create ColumnLayout object";
        emit renderFinished();
        return;
    }

    QQuickItem* layoutItem = qobject_cast<QQuickItem*>(layoutObj);
    if (!layoutItem) {
        delete layoutObj;
        emit renderFinished();
        return;
    }

    layoutItem->setParentItem(container);
    layoutItem->setWidth(container->width());
    layoutItem->setHeight(container->height());

    // Рендерим виджеты в корневой Layout
    renderWidgets(widgets, layoutItem);

    emit renderFinished();
    qDebug() << "Render complete, created" << m_widgets.size() << "widgets";
}

void JsonUiRenderer::renderWidgets(const QJsonArray& widgets, QQuickItem* parentLayout)
{
    for (const QJsonValue& value : widgets) {
        if (!value.isObject()) {
            continue;
        }

        QJsonObject spec = value.toObject();
        QString type = spec["type"].toString();
        QString id = spec["id"].toString();

        // Проверяем, является ли виджет Layout-контейнером
        bool isLayout = (type == "QVBoxLayout" || type == "QHBoxLayout" || type == "QGridLayout");

        // Создаём виджет
        QObject* widget = m_factory->create(type, spec, parentLayout);

        if (widget) {
            if (!id.isEmpty()) {
                m_widgets[id] = widget;
                emit widgetCreated(id, widget);
            }

            // Если это Layout — рекурсивно рендерим дочерние виджеты
            if (isLayout && spec.contains("widgets") && spec["widgets"].isArray()) {
                QQuickItem* containerItem = qobject_cast<QQuickItem*>(widget);
                if (containerItem) {
                    // Ищем внутренний Layout (ColumnLayout/RowLayout/GridLayout)
                    QQuickItem* innerLayout = nullptr;
                    auto children = containerItem->childItems();
                    for (QQuickItem* child : children) {
                        if (child->inherits("QQuickColumnLayout") ||
                            child->inherits("QQuickRowLayout") ||
                            child->inherits("QQuickGridLayout")) {
                            innerLayout = child;
                            break;
                        }
                    }

                    if (innerLayout) {
                        renderWidgets(spec["widgets"].toArray(), innerLayout);
                    } else {
                        // Если внутренний Layout не найден, используем сам виджет
                        renderWidgets(spec["widgets"].toArray(), containerItem);
                    }
                }
            }

            // Обработка данных для конечных виджетов
            if (!isLayout && spec.contains("data") && spec["data"].isObject()) {
                QJsonObject dataSpec = spec["data"].toObject();
                m_dataManager->requestData(id, dataSpec);
            }
        }
    }
}

QObject* JsonUiRenderer::findWidget(const QString& id) const
{
    return m_widgets.value(id, nullptr);
}

void JsonUiRenderer::updateWidgetData(const QString& id, const QJsonObject& data)
{
    QObject* widget = findWidget(id);
    if (!widget) {
        qWarning() << "Widget not found:" << id;
        return;
    }

    // TODO: Обновить данные виджета
    Q_UNUSED(data);
}

void JsonUiRenderer::clearWidgets()
{
    m_widgets.clear();
}

// ============================================================
// СЛОТЫ ДЛЯ DATA MANAGER
// ============================================================

void JsonUiRenderer::onDataReady(const QString& widgetId, const QJsonDocument& data)
{
    qDebug() << "Data ready for widget:" << widgetId;
    
    QObject* widget = findWidget(widgetId);
    if (!widget) {
        qWarning() << "Widget not found for data update:" << widgetId;
        return;
    }

    // TODO: Обновить виджет данными
    // Для графиков: передать данные в QML-компонент
    // Для таблиц: обновить модель
    // Для текстовых полей: обновить текст
}

void JsonUiRenderer::onDataError(const QString& widgetId, const QString& error, 
                                  const QString& endpoint, int httpCode)
{
    qWarning() << "Data error for widget:" << widgetId 
               << "Error:" << error
               << "Endpoint:" << endpoint
               << "HTTP:" << httpCode;
    
    // TODO: Показать сообщение об ошибке в виджете
}

void JsonUiRenderer::onDataProgress(const QString& widgetId, int percent)
{
    qDebug() << "Data progress for widget:" << widgetId << percent << "%";
    
    // TODO: Обновить прогресс-бар в виджете
}
