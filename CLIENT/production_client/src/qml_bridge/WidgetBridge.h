#ifndef WIDGETBRIDGE_H
#define WIDGETBRIDGE_H

#include <QObject>
#include <QJsonObject>
#include <QMap>
#include <QString>
#include <QQuickItem>

class QNetworkAccessManager;
class QNetworkReply;
class DataManager;
class JsonUiRenderer;

class WidgetBridge : public QObject
{
    Q_OBJECT

public:
    explicit WidgetBridge(QObject* parent = nullptr);
    ~WidgetBridge();

    DataManager* dataManager() const { return m_dataManager; }
    JsonUiRenderer* renderer() const { return m_renderer; }

public slots:
    void setDataManager(DataManager* dataManager);
    void setRenderer(JsonUiRenderer* renderer);

    void loadInterface(const QString& jsonPath);
    void loadInterfaceFromJson(const QString& jsonString);

    void requestWidgetData(const QString& widgetId, const QJsonObject& spec);
    void sendWidgetInput(const QString& widgetId, const QJsonObject& input);

    void setParameter(const QString& name, const QString& value);
    void setParameters(const QJsonObject& params);

    void refreshWidget(const QString& widgetId);
    void refreshAllWidgets();

    void httpRequest(const QString& url, const QString& method, const QString& token, const QString& body, const QString& callbackId);

    Q_INVOKABLE void renderPage(const QJsonObject& uiData, QQuickItem* container);

signals:
    void interfaceLoaded(const QString& title);
    void interfaceError(const QString& error);
    void widgetInputSent(const QString& widgetId, bool success, const QString& message);
    void httpResponse(const QString& callbackId, int status, const QString& data);

private slots:
    void onNetworkReplyFinished(QNetworkReply* reply);

private:
    QNetworkAccessManager* m_networkManager = nullptr;
    DataManager* m_dataManager = nullptr;
    JsonUiRenderer* m_renderer = nullptr;
    QJsonObject m_currentInterface;
    QMap<QNetworkReply*, QString> m_pendingRequests;
};

#endif // WIDGETBRIDGE_H
