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
#include <QVariant>
#include <functional>

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

    if (container->inherits("QQuickColumnLayout") ||
        container->inherits("QQuickRowLayout") ||
        container->inherits("QQuickGridLayout")) {
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

    if (item->inherits("QQuickColumnLayout") ||
        item->inherits("QQuickRowLayout") ||
        item->inherits("QQuickGridLayout")) {
        item->polish();
        item->update();
    } else {
        item->update();
    }

    emit layoutUpdated();
}

void JsonUiRenderer::render(const QJsonObject& root, QQuickItem* container)
{
    qDebug() << "========================================";
    qDebug() << "JsonUiRenderer::render() START";
    qDebug() << "  Container:" << (container ? "valid" : "null");
    qDebug() << "  Container width:" << (container ? container->width() : 0);
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
    m_currentFormFields.clear();
    m_currentSubmitButton = nullptr;

    QJsonArray widgets = root["widgets"].toArray();
    qDebug() << "  Total widgets in root:" << widgets.size();

    if (widgets.isEmpty()) {
        qWarning() << "WARNING: No widgets found in JSON";
        emit renderFinished();
        return;
    }

    QString layoutQml = 
        "import QtQuick 6.0\n"
        "import QtQuick.Controls 6.0\n"
        "import QtQuick.Layouts 6.0\n"
        "ColumnLayout {\n"
        "    width: parent ? parent.width : 0\n"
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
    layoutItem->setObjectName("rootLayout");

    m_lastLayout = layoutItem;

    renderWidgets(widgets, layoutItem);

    updateLayout(layoutItem);
    
    scheduleHeightUpdate(layoutItem);
    
    QTimer::singleShot(200, this, [this]() {
        refreshAllCharts();
    });

    emit renderFinished();
    qDebug() << "JsonUiRenderer::render() FINISHED (async)";
}

void JsonUiRenderer::renderWidgets(const QJsonArray& widgets, QQuickItem* parentLayout)
{
    qDebug() << "  renderWidgets() called: widgets=" << widgets.size();

    for (int i = 0; i < widgets.size(); ++i) {
        const QJsonValue& value = widgets[i];
        if (!value.isObject()) continue;

        QJsonObject spec = value.toObject();
        QString type = spec["type"].toString();
        QString id = spec["id"].toString();

        qDebug() << "    Widget[" << i << "] type=" << type << "id=" << id;

        bool hasChildren = spec.contains("widgets") && spec["widgets"].isArray();

        bool isContainer = (type == "Card" || type == "Form" || type == "QGroupBox" ||
                            type == "QVBoxLayout" || type == "QHBoxLayout" || type == "QGridLayout");

        if (type == "Form") {
            m_currentFormFields.clear();
            m_currentSubmitButton = nullptr;
            qDebug() << "      Form started - cleared form fields list";
        }

        QObject* widget = m_factory->create(type, spec, parentLayout);

        if (!widget) {
            qWarning() << "      FAILED to create widget: " << type << "id=" << id;
            continue;
        }

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
            }
        }

        if (hasChildren && targetParent) {
            renderWidgets(spec["widgets"].toArray(), targetParent);
            updateLayout(targetParent);
        }

        // ============================================================
        // Поля формы с валидацией
        // ============================================================
        if (type == "Form" && spec.contains("fields") && spec["fields"].isArray())
        {
            QJsonArray fields = spec["fields"].toArray();
            qDebug() << "      Form has" << fields.size() << "fields";
            
            if (targetParent)
            {
                for (int j = 0; j < fields.size(); ++j) {
                    QJsonObject field = fields[j].toObject();
                    QString fieldType = field["type"].toString();
                    QString fieldName = field["field"].toString();
                    QString fieldId = fieldName.isEmpty() ? (id + "_field_" + QString::number(j)) : fieldName;
                    
                    QJsonObject fieldSpec;
                    fieldSpec["id"] = fieldId;
                    fieldSpec["label"] = field["label"].toString();
                    fieldSpec["field"] = field["field"].toString();
                    fieldSpec["placeholder"] = field["placeholder"].toString();
                    fieldSpec["required"] = field["required"].toBool(false);
                    if (field.contains("validation") && !field["validation"].toString().isEmpty()) {
                        fieldSpec["validation"] = field["validation"].toString();
                    }
                    
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
                        if (field.contains("source_data")) {
                            fieldSpec["source_data"] = field["source_data"];
                        }
                    } else if (fieldType == "MultiSelectField") {
                        widgetType = "MultiSelectField";
                        fieldSpec["source"] = field["source"].toString();
                        fieldSpec["value_field"] = field["value_field"].toString();
                        fieldSpec["label_field"] = field["label_field"].toString();
                        if (field.contains("source_data")) {
                            fieldSpec["source_data"] = field["source_data"];
                        }
                    } else {
                        widgetType = "TextField";
                    }
                    
                    QObject* fieldWidget = m_factory->create(widgetType, fieldSpec, targetParent);
                    if (fieldWidget)
                    {
                        QQuickItem* fieldItem = qobject_cast<QQuickItem*>(fieldWidget);
                        if (fieldItem) {
                            fieldItem->setParentItem(targetParent);

                            // Ищем внутри созданного виджета тот Item,
                            // у которого есть property "fieldIsValid".
                            QQuickItem* validatable = nullptr;
                            std::function<void(QQuickItem*)> findValidatable =
                                [&](QQuickItem* item) {
                                    if (!item || validatable) return;
                                    if (item->property("fieldIsValid").isValid()) {
                                        validatable = item;
                                        return;
                                    }
                                    for (QQuickItem* child : item->childItems()) {
                                        findValidatable(child);
                                    }
                                };
                            findValidatable(fieldItem);

                            if (validatable) {
                                m_currentFormFields.append(validatable);
                                QObject::connect(validatable, SIGNAL(fieldValidationChanged(bool)),
                                                 this, SLOT(onFieldValidationChanged(bool)));
                                qDebug() << "        Connected validatable field:" << fieldId
                                         << "class=" << validatable->metaObject()->className();
                            } else {
                                qWarning() << "        No validatable item found for field:" << fieldId;
                            }
                        }
                    }
                }
                updateLayout(targetParent);
            }
        }

        // ============================================================
        // Submit-кнопка формы
        // ============================================================
        if (type == "Form" && spec.contains("submit_endpoint") && targetParent)
        {
            QString submitEndpoint = spec["submit_endpoint"].toString();
            QString submitMethod = spec["submit_method"].toString("POST");
            
            QJsonObject buttonSpec;
            buttonSpec["type"] = "QPushButton";
            buttonSpec["id"] = id + "_submit";
            buttonSpec["text"] = spec["submit_text"].toString("Создать");
            buttonSpec["form_endpoint"] = submitEndpoint;
            buttonSpec["form_method"] = submitMethod;
            buttonSpec["form_fields"] = spec["fields"];
            
            QObject* submitButton = m_factory->create("QPushButton", buttonSpec, targetParent);
            if (submitButton)
            {
                QQuickItem* buttonItem = qobject_cast<QQuickItem*>(submitButton);
                if (buttonItem)
                {
                    buttonItem->setParentItem(targetParent);
                    m_currentSubmitButton = buttonItem;
                    qDebug() << "      Submit button created";
                    updateSubmitButtonState();
                }
            }
            updateLayout(targetParent);
        }

        if (!isContainer && spec.contains("data") && spec["data"].isObject()) {
            QJsonObject dataSpec = spec["data"].toObject();
            m_dataManager->requestData(id, dataSpec);
        }
    }
}

void JsonUiRenderer::updateSubmitButtonState()
{
    if (!m_currentSubmitButton) {
        return;
    }

    bool allValid = true;
    for (QQuickItem* item : m_currentFormFields) {
        if (!item) continue;
        QVariant v = item->property("fieldIsValid");
        if (v.isValid() && !v.toBool()) {
            allValid = false;
            break;
        }
    }

    m_currentSubmitButton->setProperty("enabled", allValid);
    qDebug() << "updateSubmitButtonState: allValid=" << allValid
             << "fields=" << m_currentFormFields.size();
}

void JsonUiRenderer::onFieldValidationChanged(bool valid)
{
    qDebug() << "onFieldValidationChanged: valid=" << valid;
    updateSubmitButtonState();
}

QObject* JsonUiRenderer::findWidget(const QString& id) const
{
    return m_widgets.value(id, nullptr);
}

void JsonUiRenderer::updateWidgetData(const QString& id, const QJsonObject& data)
{
    Q_UNUSED(id);
    Q_UNUSED(data);
}

void JsonUiRenderer::clearWidgets()
{
    m_widgets.clear();
    m_currentFormFields.clear();
    m_currentSubmitButton = nullptr;
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
    }
}

void JsonUiRenderer::refreshAllWidgets()
{
    if (m_lastLayout) {
        QList<QQuickItem*> items = m_lastLayout->childItems();
        for (QQuickItem* item : items) {
            if (item) {
                item->polish();
                item->update();
            }
        }
        m_lastLayout->polish();
        m_lastLayout->update();
        scheduleHeightUpdate(m_lastLayout);
    }
}
