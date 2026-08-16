#ifndef JSONUIRENDERER_H
#define JSONUIRENDERER_H

#include <QObject>
#include <QJsonObject>
#include <QMap>

class QQmlEngine;
class QQuickItem;
class QmlObjectFactory;
class DataManager;

class JsonUiRenderer : public QObject
{
    Q_OBJECT

public:
    explicit JsonUiRenderer(QQmlEngine* engine, QObject* parent = nullptr);
    ~JsonUiRenderer();

    void render(const QJsonObject& root, QQuickItem* container);
    QObject* findWidget(const QString& id) const;
    void updateWidgetData(const QString& id, const QJsonObject& data);
    int widgetCount() const { return m_widgets.size(); }
    void clearWidgets();

    DataManager* dataManager() const { return m_dataManager; }

signals:
    void widgetCreated(const QString& id, QObject* widget);
    void renderStarted();
    void renderFinished();

private slots:
    void onDataReady(const QString& widgetId, const QJsonDocument& data);
    void onDataError(const QString& widgetId, const QString& error, const QString& endpoint, int httpCode);
    void onDataProgress(const QString& widgetId, int percent);

private:
    void renderWidgets(const QJsonArray& widgets, QQuickItem* parentLayout);

    QQmlEngine* m_engine;
    QmlObjectFactory* m_factory;
    DataManager* m_dataManager;
    QMap<QString, QObject*> m_widgets;
};

#endif // JSONUIRENDERER_H
