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
    // userId используется ТОЛЬКО для BUS-запросов (X-User-ID).
    // Для MG-запросов X-User-ID не нужен (nginx ставит сам).
    void setUserId(int userId);

    // MG Service (REST)
    void loadMastergraph(const QString& mgId);
    void checkAccess(const QString& mgId);
    void loadHolidays();

    // BUS (REST) — все запросы ставят X-User-ID (m_userId)
    void createLocalGraph(const QString& taskId, const QString& busUrl, const QVariantMap& payload);
    void recalcForecast(const QString& taskId, const QString& busUrl, const QVariantMap& payload);
    void loadLocalGraph(const QString& taskId, const QString& busUrl);

    // BUS: ЛГ-зависимости
    void loadLgDependencies(const QString& lgId, const QString& busUrl);
    void createLgDependency(const QString& lgId, const QString& busUrl,
                            const QString& predecessorId, const QString& successorId);
    void deleteLgDependency(const QString& lgId, const QString& busUrl, const QString& depId);

signals:
    void mastergraphLoaded(const QVariantMap& data);
    void accessLoaded(bool canEdit, bool canView, const QString& reason);
    void holidaysLoaded(const QVariantList& holidays);
    void localGraphCreated(const QVariantMap& result);
    void forecastRecalculated(const QVariantMap& result);
    void localGraphLoaded(const QVariantMap& lgData, const QVariantList& tasks);
    void lgDependenciesLoaded(const QVariantList& dependencies);
    void lgDependencyCreated(const QVariantMap& result);
    void lgDependencyDeleted(const QString& depId);
    void errorOccurred(const QString& message);

private:
    QNetworkAccessManager* m_manager;
    QString m_baseUrl;
    QString m_token;
    int m_userId = 0;

    // Ставит X-User-ID, если m_userId > 0. Только для BUS-запросов.
    void applyUserIdHeader(QNetworkRequest& request) const;
};

#endif // GANTTAPICLIENT_H
