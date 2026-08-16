#ifndef JSONUIRENDERER_H
#define JSONUIRENDERER_H

#include <QObject>
#include <QJsonObject>
#include <QMap>

class QQmlEngine;
class QQuickItem;
class QmlObjectFactory;

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

signals:
    void widgetCreated(const QString& id, QObject* widget);
    void renderStarted();
    void renderFinished();

private:
    void renderWidgets(const QJsonArray& widgets, QQuickItem* parentLayout);

    QQmlEngine* m_engine;
    QmlObjectFactory* m_factory;
    QMap<QString, QObject*> m_widgets;
};

#endif // JSONUIRENDERER_H
