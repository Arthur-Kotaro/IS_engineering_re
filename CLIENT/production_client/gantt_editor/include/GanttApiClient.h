#ifndef GANTTAPICLIENT_H
#define GANTTAPICLIENT_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QVariantList>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class GanttApiClient : public QObject
{
    Q_OBJECT
public:
    explicit GanttApiClient(QObject *parent = nullptr);

    void setBaseUrl(const QString& url);
    void setToken(const QString& token);

    // MG Service (REST)
    void loadMastergraph(const QString& mgId);
    void checkAccess(const QString& mgId);
    void loadHolidays();

    // BUS (REST)
    void createLocalGraph(const QString& taskId, const QString& busUrl, const QVariantMap& payload);
    void recalcForecast(const QString& taskId, const QString& busUrl, const QVariantMap& payload);
    
    // BUS: Load Local Graph
    void loadLocalGraph(const QString& taskId, const QString& busUrl);

signals:
    void mastergraphLoaded(const QVariantMap& data);
    void accessLoaded(bool canEdit, bool canView, const QString& reason);
    void holidaysLoaded(const QVariantList& holidays);
    void localGraphCreated(const QVariantMap& result);
    void forecastRecalculated(const QVariantMap& result);
    void localGraphLoaded(const QVariantMap& lgData, const QVariantList& tasks);
    void errorOccurred(const QString& message);

private:
    QNetworkAccessManager* m_manager;
    QString m_baseUrl;
    QString m_token;
};

#endif // GANTTAPICLIENT_H
