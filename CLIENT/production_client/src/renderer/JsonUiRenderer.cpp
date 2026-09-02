#include "JsonUiRenderer.h"
#include "QmlObjectFactory.h"
#include "core/DataManager.h"
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQuickItem>
#include <QDebug>
#include <QJsonArray>
#include <QJsonValue>
#include <QTimer>
#include <QMetaObject>

JsonUiRenderer::JsonUiRenderer(QQmlEngine* engine, QObject* parent)
    : QObject(parent)
    , m_engine(engine)
    , m_factory(new QmlObjectFactory(engine, this))
    , m_dataManager(new DataManager(this))
{
    qDebug() << "========================================";
    qDebug() << "JsonUiRenderer::JsonUiRenderer()";
    qDebug() << "  Engine:" << (engine ? "valid" : "null");
    qDebug() << "  Factory:" << (m_factory ? "created" : "null");
    qDebug() << "  DataManager:" << (m_dataManager ? "created" : "null");
    qDebug() << "========================================";

    connect(m_dataManager, &DataManager::dataReady,
            this, &JsonUiRenderer::onDataReady);
    connect(m_dataManager, &DataManager::dataError,
            this, &JsonUiRenderer::onDataError);
    connect(m_dataManager, &DataManager::dataProgress,
            this, &JsonUiRenderer::onDataProgress);
}

JsonUiRenderer::~JsonUiRenderer()
{
    qDebug() << "JsonUiRenderer::~JsonUiRenderer() - widgets count:" << m_widgets.size();
}

int JsonUiRenderer::getContentHeight(QQuickItem* item)
{
    if (!item) return 0;
    
    QRectF rect = item->childrenRect();
    int height = static_cast<int>(rect.height()) + 40;
    
    qDebug() << "getContentHeight():" << height << "childrenRect.height:" << rect.height();
    
    return height;
}

void JsonUiRenderer::scheduleHeightUpdate(QQuickItem* layoutItem)
{
    if (!layoutItem) return;
    
    QTimer::singleShot(0, this, [this, layoutItem]() {
        if (!layoutItem) return;
        
        layoutItem->polish();
        layoutItem->update();
        
        QList<QQuickItem*> childItems = layoutItem->childItems();
        for (QQuickItem* child : childItems) {
            if (child) {
                child->polish();
                child->update();
            }
        }
        
        int height = getContentHeight(layoutItem);
        qDebug() << "scheduleHeightUpdate: contentHeight =" << height;
        emit contentHeightChanged(height);
    });
}

void JsonUiRenderer::refreshContentHeight()
{
    if (m_lastLayout) {
        scheduleHeightUpdate(m_lastLayout);
    }
}

void JsonUiRenderer::repaintCharts(QQuickItem* item)
{
    if (!item) return;
    
    auto children = item->childItems();
    for (QQuickItem* child : children) {
        if (child) {
            if (child->inherits("QQuickCanvasItem")) {
                qDebug() << "Repainting Canvas:" << child->objectName();
                QMetaObject::invokeMethod(child, "requestPaint", Qt::QueuedConnection);
            }
            repaintCharts(child);
        }
    }
}

void JsonUiRenderer::refreshAllCharts()
{
    if (m_lastLayout) {
        qDebug() << "refreshAllCharts: forcing repaint of all charts";
        repaintCharts(m_lastLayout);
        scheduleHeightUpdate(m_lastLayout);
    }
}

QQuickItem* JsonUiRenderer::findInnerLayout(QQuickItem* container, int depth)
{
    if (!container) return nullptr;

    qDebug() << "    findInnerLayout() depth=" << depth << "class=" << container->metaObject()->className();

    if (container->inherits("QQuickColumnLayout") ||
        container->inherits("QQuickRowLayout") ||
        container->inherits("QQuickGridLayout")) {
        qDebug() << "      Found Layout!";
        return container;
    }

    auto children = container->childItems();
    for (QQuickItem* child : children) {
        QQuickItem* result = findInnerLayout(child, depth + 1);
        if (result) {
            return result;
        }
    }

    return nullptr;
}

void JsonUiRenderer::updateLayout(QQuickItem* item)
{
    if (!item) return;

    QString className = item->metaObject()->className();
    qDebug() << "updateLayout() called for" << className;

    if (item->inherits("QQuickColumnLayout") ||
        item->inherits("QQuickRowLayout") ||
        item->inherits("QQuickGridLayout")) {
        item->polish();
        item->update();
        qDebug() << "  Layout updated with polish() + update()";
    } else {
        item->update();
        qDebug() << "  Container updated with update()";
    }

    emit layoutUpdated();
}

void JsonUiRenderer::render(const QJsonObject& root, QQuickItem* container)
{
    qDebug() << "========================================";
    qDebug() << "JsonUiRenderer::render() START";
    qDebug() << "  Container:" << (container ? "valid" : "null");
    qDebug() << "  Container width:" << (container ? container->width() : 0);
    qDebug() << "  Container height:" << (container ? container->height() : 0);
    qDebug() << "========================================";

    emit renderStarted();

    if (!container) {
        qWarning() << "ERROR: Container is null!";
        emit renderFinished();
        return;
    }

    auto children = container->childItems();
    for (QQuickItem* child : children) {
        child->deleteLater();
    }
    m_widgets.clear();

    QJsonArray widgets = root["widgets"].toArray();
    qDebug() << "  Total widgets in root:" << widgets.size();

    if (widgets.isEmpty()) {
        qWarning() << "WARNING: No widgets found in JSON";
        emit renderFinished();
        return;
    }

    for (int i = 0; i < widgets.size(); ++i) {
        QJsonObject obj = widgets[i].toObject();
        qDebug() << "  Root widget[" << i << "]:" << obj["type"].toString() << "id=" << obj["id"].toString();
    }

    qDebug() << "Creating root ColumnLayout...";

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
        qWarning() << "ERROR: Failed to create ColumnLayout:" << layoutComponent.errorString();
        emit renderFinished();
        return;
    }

    QObject* layoutObj = layoutComponent.create();
    if (!layoutObj) {
        qWarning() << "ERROR: Failed to create ColumnLayout object";
        emit renderFinished();
        return;
    }

    QQuickItem* layoutItem = qobject_cast<QQuickItem*>(layoutObj);
    if (!layoutItem) {
        qWarning() << "ERROR: Layout object is not QQuickItem";
        delete layoutObj;
        emit renderFinished();
        return;
    }

    layoutItem->setParentItem(container);
    layoutItem->setWidth(container->width());
    layoutItem->setHeight(container->height());

    qDebug() << "  Root ColumnLayout created, width=" << layoutItem->width() << "height=" << layoutItem->height();

    m_lastLayout = layoutItem;

    renderWidgets(widgets, layoutItem);

    updateLayout(layoutItem);
    
    scheduleHeightUpdate(layoutItem);
    
    QTimer::singleShot(200, this, [this]() {
        refreshAllCharts();
    });

    emit renderFinished();
    qDebug() << "========================================";
    qDebug() << "JsonUiRenderer::render() FINISHED (async)";
    qDebug() << "========================================";
}

void JsonUiRenderer::renderWidgets(const QJsonArray& widgets, QQuickItem* parentLayout)
{
    qDebug() << "  renderWidgets() called: widgets=" << widgets.size() << ", parentLayout=" << (parentLayout ? "valid" : "null");
    qDebug() << "    ParentLayout width=" << (parentLayout ? parentLayout->width() : 0) << "height=" << (parentLayout ? parentLayout->height() : 0);

    for (int i = 0; i < widgets.size(); ++i) {
        const QJsonValue& value = widgets[i];
        if (!value.isObject()) {
            qDebug() << "    Widget[" << i << "] is not an object, skipping";
            continue;
        }

        QJsonObject spec = value.toObject();
        QString type = spec["type"].toString();
        QString id = spec["id"].toString();

        qDebug() << "    Widget[" << i << "] type=" << type << "id=" << id;

        if (spec.contains("height")) {
            qDebug() << "      height=" << spec["height"].toInt();
        } else {
            qDebug() << "      WARNING: no 'height' field";
        }

        bool hasChildren = spec.contains("widgets") && spec["widgets"].isArray();
        bool hasFields = false;
        
        if (spec.contains("data") && spec["data"].isObject()) {
            QJsonObject dataObj = spec["data"].toObject();
            if (dataObj.contains("fields") && dataObj["fields"].isArray()) {
                hasFields = true;
            }
        }
        
        if (hasChildren) {
            qDebug() << "      widgets count=" << spec["widgets"].toArray().size();
        }
        if (hasFields) {
            qDebug() << "      fields count in data";
        }

        bool isContainer = (type == "Card" || type == "Form" || type == "QGroupBox" ||
                            type == "QVBoxLayout" || type == "QHBoxLayout" || type == "QGridLayout");

        if (isContainer) {
            qDebug() << "      This is a CONTAINER widget";
        }

        QObject* widget = m_factory->create(type, spec, parentLayout);

        if (widget) {
            qDebug() << "      Widget CREATED successfully: " << type << "id=" << id;
            if (!id.isEmpty()) {
                m_widgets[id] = widget;
                emit widgetCreated(id, widget);
            }

            QQuickItem* containerItem = qobject_cast<QQuickItem*>(widget);
            QQuickItem* targetParent = containerItem;
            
            if (containerItem) {
                QQuickItem* innerLayout = findInnerLayout(containerItem, 0);
                if (innerLayout) {
                    targetParent = innerLayout;
                    qDebug() << "      Found inner layout for children";
                }
            }

            // Обработка дочерних виджетов (widgets)
            if (hasChildren && targetParent) {
                int childCount = spec["widgets"].toArray().size();
                qDebug() << "      Container has" << childCount << "child widgets";
                renderWidgets(spec["widgets"].toArray(), targetParent);
                updateLayout(targetParent);
            }

            // Обработка полей (fields) из data для Card
            if (hasFields && (type == "Card" || type == "QGroupBox")) {
                QJsonObject dataObj = spec["data"].toObject();
                QJsonArray fields = dataObj["fields"].toArray();
                qDebug() << "      Card has" << fields.size() << "fields";
                
                if (targetParent) {
                    for (int j = 0; j < fields.size(); ++j) {
                        QJsonObject field = fields[j].toObject();
                        QString label = field["label"].toString();
                        QString value = field["value"].toString();
                        QString fieldId = id + "_field_" + QString::number(j);
                        
                        qDebug() << "        Field[" << j << "] label=" << label << "value=" << value;
                        
                        QJsonObject fieldSpec;
                        fieldSpec["type"] = "QLabel";
                        fieldSpec["id"] = fieldId;
                        fieldSpec["text"] = label + ": " + value;
                        
                        QObject* fieldWidget = m_factory->create("QLabel", fieldSpec, targetParent);
                        if (fieldWidget) {
                            QQuickItem* fieldItem = qobject_cast<QQuickItem*>(fieldWidget);
                            if (fieldItem) {
                                fieldItem->setParentItem(targetParent);
                                qDebug() << "        Field widget created: " << fieldId;
                            }
                        }
                    }
                    updateLayout(targetParent);
                }
            }

            // Обработка полей (fields) для Form
            if (type == "Form" && spec.contains("fields") && spec["fields"].isArray()) {
                QJsonArray fields = spec["fields"].toArray();
                qDebug() << "      Form has" << fields.size() << "fields";
                
                if (targetParent) {
                    for (int j = 0; j < fields.size(); ++j) {
                        QJsonObject field = fields[j].toObject();
                        QString fieldType = field["type"].toString();
                        QString fieldId = id + "_field_" + QString::number(j);
                        
                        qDebug() << "        Field[" << j << "] type=" << fieldType << " field=" << field["field"].toString();
                        
                        QJsonObject fieldSpec;
                        fieldSpec["id"] = fieldId;
                        fieldSpec["label"] = field["label"].toString();
                        fieldSpec["field"] = field["field"].toString();
                        fieldSpec["placeholder"] = field["placeholder"].toString();
                        fieldSpec["required"] = field["required"].toBool(false);
                        
                        QString widgetType;
                        if (fieldType == "EmailField") {
                            widgetType = "EmailField";
                        } else if (fieldType == "PasswordField") {
                            widgetType = "PasswordField";
                        } else if (fieldType == "SelectField") {
                            widgetType = "SelectField";
                            fieldSpec["source"] = field["source"].toString();
                            fieldSpec["value_field"] = field["value_field"].toString();
                            fieldSpec["label_field"] = field["label_field"].toString();
                        } else if (fieldType == "MultiSelectField") {
                            widgetType = "MultiSelectField";
                            fieldSpec["source"] = field["source"].toString();
                            fieldSpec["value_field"] = field["value_field"].toString();
                            fieldSpec["label_field"] = field["label_field"].toString();
                        } else {
                            widgetType = "TextField";
                        }
                        
                        QObject* fieldWidget = m_factory->create(widgetType, fieldSpec, targetParent);
                        if (fieldWidget) {
                            QQuickItem* fieldItem = qobject_cast<QQuickItem*>(fieldWidget);
                            if (fieldItem) {
                                fieldItem->setParentItem(targetParent);
                                qDebug() << "        Field widget created: " << fieldId;
                            }
                        }
                    }
                    updateLayout(targetParent);
                }
            }

            // Кнопка Submit для Form
            if (type == "Form" && spec.contains("submit_endpoint") && targetParent) {
                QString submitEndpoint = spec["submit_endpoint"].toString();
                QString submitMethod = spec["submit_method"].toString("POST");
                
                QJsonObject buttonSpec;
                buttonSpec["type"] = "QPushButton";
                buttonSpec["id"] = id + "_submit";
                buttonSpec["text"] = spec["submit_text"].toString("Создать");
                
                QObject* submitButton = m_factory->create("QPushButton", buttonSpec, targetParent);
                if (submitButton) {
                    QQuickItem* buttonItem = qobject_cast<QQuickItem*>(submitButton);
                    if (buttonItem) {
                        buttonItem->setParentItem(targetParent);
                        qDebug() << "      Submit button created for Form: " << id;
                    }
                }
                updateLayout(targetParent);
            }

            if (!isContainer && spec.contains("data") && spec["data"].isObject()) {
                QJsonObject dataSpec = spec["data"].toObject();
                qDebug() << "      Data source detected, requesting data";
                m_dataManager->requestData(id, dataSpec);
            }
        } else {
            qWarning() << "      FAILED to create widget: " << type << "id=" << id;
        }
    }

    qDebug() << "  renderWidgets() completed";
}

QObject* JsonUiRenderer::findWidget(const QString& id) const
{
    QObject* widget = m_widgets.value(id, nullptr);
    qDebug() << "findWidget(" << id << ") ->" << (widget ? "found" : "not found");
    return widget;
}

void JsonUiRenderer::updateWidgetData(const QString& id, const QJsonObject& data)
{
    qDebug() << "updateWidgetData(" << id << ") called, data size=" << data.size();
    QObject* widget = findWidget(id);
    if (!widget) {
        qWarning() << "  Widget not found:" << id;
        return;
    }
    qDebug() << "  Widget found, updating...";
}

void JsonUiRenderer::clearWidgets()
{
    qDebug() << "clearWidgets() called, clearing" << m_widgets.size() << "widgets";
    m_widgets.clear();
}

void JsonUiRenderer::onDataReady(const QString& widgetId, const QJsonDocument& data)
{
    qDebug() << "DATA READY for widget:" << widgetId << "data size=" << data.toJson().size();
    if (m_lastLayout) {
        scheduleHeightUpdate(m_lastLayout);
        QTimer::singleShot(100, this, [this]() {
            refreshAllCharts();
        });
    }
}

void JsonUiRenderer::onDataError(const QString& widgetId, const QString& error, 
                                  const QString& endpoint, int httpCode)
{
    qWarning() << "DATA ERROR for widget:" << widgetId 
               << "Error:" << error
               << "Endpoint:" << endpoint
               << "HTTP:" << httpCode;
}

void JsonUiRenderer::onDataProgress(const QString& widgetId, int percent)
{
    qDebug() << "DATA PROGRESS for widget:" << widgetId << percent << "%";
}

void JsonUiRenderer::setDataManager(DataManager* dataManager)
{
    if (m_dataManager != dataManager) {
        m_dataManager = dataManager;
        qDebug() << "JsonUiRenderer: DataManager set";
    }
}
