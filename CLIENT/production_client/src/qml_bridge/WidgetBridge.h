#ifndef WIDGETBRIDGE_H
#define WIDGETBRIDGE_H

#include <QObject>
#include <QJsonObject>
#include <QMap>

class DataManager;
class JsonUiRenderer;

class WidgetBridge : public QObject
{
    Q_OBJECT

public:
    explicit WidgetBridge(QObject* parent = nullptr);
    ~WidgetBridge();

    // Геттеры/сеттеры для C++ (не экспортируются в QML)
    DataManager* dataManager() const { return m_dataManager; }
    void setDataManager(DataManager* dataManager);

    JsonUiRenderer* renderer() const { return m_renderer; }
    void setRenderer(JsonUiRenderer* renderer);

public slots:
    void loadInterface(const QString& jsonPath);
    void loadInterfaceFromJson(const QString& jsonString);
    void requestWidgetData(const QString& widgetId, const QJsonObject& spec);
    void sendWidgetInput(const QString& widgetId, const QJsonObject& input);
    void setParameter(const QString& name, const QString& value);
    void setParameters(const QJsonObject& params);
    void refreshWidget(const QString& widgetId);
    void refreshAllWidgets();

signals:
    void dataManagerChanged();
    void rendererChanged();
    void interfaceLoaded(const QString& title);
    void interfaceError(const QString& error);
    void widgetInputSent(const QString& widgetId, bool success, const QString& message);
    void widgetUpdated(const QString& widgetId);

private:
    DataManager* m_dataManager = nullptr;
    JsonUiRenderer* m_renderer = nullptr;
    QJsonObject m_currentInterface;
};

#endif // WIDGETBRIDGE_H
