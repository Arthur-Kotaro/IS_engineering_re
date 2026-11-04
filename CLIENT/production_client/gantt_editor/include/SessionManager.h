#ifndef SESSIONMANAGER_H
#define SESSIONMANAGER_H

#include <QObject>
#include <QHash>
#include <QString>
#include <QQmlApplicationEngine>
#include <QPointer>

class ProjectController;

class SessionManager : public QObject
{
    Q_OBJECT
public:
    explicit SessionManager(QObject *parent = nullptr);
    ~SessionManager();

    void registerMasterSession(QQmlApplicationEngine* engine, ProjectController* controller);

    Q_INVOKABLE bool openLocalGraphSession(const QString& taskId);
    Q_INVOKABLE void closeLocalGraphSession(const QString& taskId);
    Q_INVOKABLE bool isLocalGraphSessionOpen(const QString& taskId) const;
    Q_INVOKABLE void notifyWindowClosed(const QString& taskId);

signals:
    void localGraphSessionClosed(const QString& taskId);
    void localGraphSaved(const QString& taskId, const QString& filePath);

private:
    struct LocalSession {
        QQmlApplicationEngine* engine = nullptr;
        ProjectController* controller = nullptr;
        QObject* window = nullptr;
        QString taskId;
    };

    QQmlApplicationEngine* m_masterEngine = nullptr;
    QPointer<ProjectController> m_masterController;

    QHash<QString, LocalSession> m_localSessions;
};

#endif
