#ifndef WIDGETBRIDGE_H
#define WIDGETBRIDGE_H

#include <QJsonObject>
#include <QMap>
#include <QObject>
#include <QQuickItem>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;
class DataManager;
class JsonUiRenderer;

class WidgetBridge : public QObject
{
    Q_OBJECT

  public:
    explicit WidgetBridge(QObject *parent = nullptr);
    ~WidgetBridge();

    DataManager *dataManager() const
    {
        return m_dataManager;
    }
    JsonUiRenderer *renderer() const
    {
        return m_renderer;
    }

  public slots:
    void setDataManager(DataManager *dataManager);
    void setRenderer(JsonUiRenderer *renderer);

    void loadInterface(const QString &jsonPath);
    void loadInterfaceFromJson(const QString &jsonString);

    void requestWidgetData(const QString &widgetId, const QJsonObject &spec);
    void sendWidgetInput(const QString &widgetId, const QJsonObject &input);

    void setParameter(const QString &name, const QString &value);
    void setParameters(const QJsonObject &params);

    void refreshWidget(const QString &widgetId);
    void refreshAllWidgets();

    void httpRequest(const QString &url, const QString &method, const QString &token,
                     const QString &body, const QString &callbackId);

    Q_INVOKABLE void renderPage(const QJsonObject &uiData, QQuickItem *container);

    Q_INVOKABLE void setAccessToken(const QString &token);
    Q_INVOKABLE QString getParameter(const QString &name) const;
    Q_INVOKABLE void submitForm(const QString &endpoint, const QString &method,
                                const QJsonObject &body, const QString &callbackId);

    Q_INVOKABLE void setCurrentSessionId(const QString &sessionId);
    Q_INVOKABLE QString currentSessionId() const
    {
        return m_currentSessionId;
    }
    Q_INVOKABLE void clearParameters();

    Q_INVOKABLE void sendWorkflowEvent(const QString &widgetId, const QString &eventType,
                                       const QJsonObject &data);

  signals:
    void interfaceLoaded(const QString &title);
    void interfaceError(const QString &error);
    void widgetInputSent(const QString &widgetId, bool success, const QString &message);
    void httpResponse(const QString &callbackId, int status, const QString &data);
    void workflowEventCompleted(const QJsonObject &response);

  private slots:
    void onNetworkReplyFinished(QNetworkReply *reply);

  private:
    QJsonObject normalizeArrayValues(const QJsonObject &body);

    QNetworkAccessManager *m_networkManager = nullptr;
    DataManager *m_dataManager = nullptr;
    JsonUiRenderer *m_renderer = nullptr;
    QJsonObject m_currentInterface;
    QMap<QNetworkReply *, QString> m_pendingRequests;
    QMap<QNetworkReply *, bool> m_workflowRequests;
    QString m_currentToken;
    QString m_currentSessionId;
};

#endif // WIDGETBRIDGE_H
