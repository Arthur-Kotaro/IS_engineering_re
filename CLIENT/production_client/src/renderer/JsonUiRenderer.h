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

    Q_INVOKABLE void render(const QJsonObject& root, QQuickItem* container);
    Q_INVOKABLE void updateWidgetData(const QString& id, const QJsonObject& data);
    Q_INVOKABLE void refreshAllCharts();
    Q_INVOKABLE void refreshContentHeight();
    Q_INVOKABLE void refreshAllWidgets();
    Q_INVOKABLE int getContentHeight(QQuickItem* item);

    void setDataManager(DataManager* dataManager);
    DataManager* dataManager() const { return m_dataManager; }

    QObject* findWidget(const QString& id) const;
    void clearWidgets();
    void updateLayout(QQuickItem* item);

signals:
    void widgetCreated(const QString& id, QObject* widget);
    void renderStarted();
    void renderFinished();
    void layoutUpdated();
    void contentHeightChanged(int height);

private slots:
    void onDataReady(const QString& widgetId, const QJsonDocument& data);
    void onDataError(const QString& widgetId, const QString& error, const QString& endpoint, int httpCode);
    void onDataProgress(const QString& widgetId, int percent);

private:
    void renderWidgets(const QJsonArray& widgets, QQuickItem* parentLayout);
    QQuickItem* findInnerLayout(QQuickItem* container, int depth = 0);
    void scheduleHeightUpdate(QQuickItem* layoutItem);
    void repaintCharts(QQuickItem* item);

    QQmlEngine* m_engine;
    QmlObjectFactory* m_factory;
    DataManager* m_dataManager;
    QMap<QString, QObject*> m_widgets;
    QQuickItem* m_lastLayout = nullptr;
};

#endif // JSONUIRENDERER_H
