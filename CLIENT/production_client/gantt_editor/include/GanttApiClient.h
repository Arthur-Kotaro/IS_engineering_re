#ifndef GANTTAPICLIENT_H
#define GANTTAPICLIENT_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class GanttApiClient : public QObject
{
    Q_OBJECT
public:
    explicit GanttApiClient(QObject *parent = nullptr);

    void setBaseUrl(const QString& url);
    void setToken(const QString& token);

    // GET /api/v1/mastergraphs/{mg_id}
    void loadMastergraph(const QString& mgId);

    // GET /api/v1/mastergraphs/{mg_id}/access
    void checkAccess(const QString& mgId);
    void loadHolidays();

signals:
    void mastergraphLoaded(const QVariantMap& data);
    void accessLoaded(bool canEdit, bool canView, const QString& reason);
    void errorOccurred(const QString& message);
    void holidaysLoaded(const QVariantList& holidays);

private:
    QNetworkAccessManager* m_manager;
    QString m_baseUrl;
    QString m_token;
};

#endif // GANTTAPICLIENT_H
