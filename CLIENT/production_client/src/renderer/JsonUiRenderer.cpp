#include "JsonUiRenderer.h"
#include "QmlObjectFactory.h"
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
{
    qDebug() << "JsonUiRenderer initialized";
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

    // Создаём ColumnLayout внутри контейнера
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

    // Рендерим виджеты в этот Layout
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

        // Создаём виджет
        QObject* widget = m_factory->create(type, spec, parentLayout);

        if (widget) {
            if (!id.isEmpty()) {
                m_widgets[id] = widget;
                emit widgetCreated(id, widget);
            }

            // Если это контейнер (QGroupBox, QHBoxLayout) — рендерим дочерние виджеты
            if ((type == "QGroupBox" || type == "QHBoxLayout") && spec.contains("widgets") && spec["widgets"].isArray()) {
                QQuickItem* containerItem = qobject_cast<QQuickItem*>(widget);
                if (containerItem) {
                    // Ищем первый дочерний Layout внутри GroupBox
                    QQuickItem* childLayout = nullptr;
                    auto children = containerItem->childItems();
                    for (QQuickItem* child : children) {
                        // Ищем ColumnLayout или RowLayout
                        if (child->property("objectName").toString().isEmpty()) {
                            childLayout = child;
                            break;
                        }
                    }

                    // Если нашли Layout, рендерим в него
                    if (childLayout) {
                        renderWidgets(spec["widgets"].toArray(), childLayout);
                    } else {
                        // Если Layout не найден, используем сам контейнер
                        renderWidgets(spec["widgets"].toArray(), containerItem);
                    }
                }
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

    Q_UNUSED(data);
}
