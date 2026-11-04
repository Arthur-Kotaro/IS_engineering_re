#include "SessionManager.h"
#include "ProjectController.h"
#include <QQmlComponent>
#include <QQmlContext>
#include <QQuickWindow>
#include <QDebug>
#include <QFile>

SessionManager::SessionManager(QObject *parent) : QObject(parent) {}

SessionManager::~SessionManager() {}

void SessionManager::registerMasterSession(QQmlApplicationEngine* engine, ProjectController* controller)
{
    m_masterEngine = engine;
    m_masterController = controller;
}

bool SessionManager::isLocalGraphSessionOpen(const QString& taskId) const
{
    return m_localSessions.contains(taskId);
}

bool SessionManager::openLocalGraphSession(const QString& taskId)
{
    if (!m_masterController)
    {
        qWarning() << "SessionManager: no master session registered";
        return false;
    }

    if (m_localSessions.contains(taskId))
    {
        LocalSession& session = m_localSessions[taskId];
        if (session.window)
        {
            QQuickWindow* qw = qobject_cast<QQuickWindow*>(session.window);
            if (qw) { qw->show(); qw->raise(); qw->requestActivate(); }
        }
        return true;
    }

    QString path = m_masterController->get_localGraphController()->getLocalGraphPath(taskId);
    if (path.isEmpty() || !QFile::exists(path))
    {
        qWarning() << "SessionManager: local graph file not found for task" << taskId;
        return false;
    }

    QQmlApplicationEngine* engine = new QQmlApplicationEngine();
    ProjectController* controller = new ProjectController();

    if (!controller->openLocalGraphFile(path))
    {
        qWarning() << "SessionManager: failed to open local graph file" << path;
        delete controller;
        delete engine;
        return false;
    }

    // Путь A: при каждом открытии ЛГ передаём актуальные границы из МГ.
    if (m_masterController && m_masterController->get_projectData())
    {
        ProjectData* master = m_masterController->get_projectData();
        QVariantMap masterTask = master->get_taskModel()->getTask(taskId);

        if (!masterTask.isEmpty())
        {
            QDate targetStart = masterTask["startDate"].toDate();
            QDate targetEnd   = masterTask["endDate"].toDate();

            if (targetStart.isValid())
                controller->get_projectData()->set_LinkedTargetStart(targetStart);
            if (targetEnd.isValid())
                controller->get_projectData()->set_LinkedTargetEnd(targetEnd);

            // Прогнозная дата завершения непосредственного предшественника (fallback — целевая).
            QStringList predIds = master->get_dependencyModel()->getPredecessors(taskId);
            QDate maxPredEnd;
            for (const QString& pid : predIds)
            {
                QVariantMap pt = master->get_taskModel()->getTask(pid);
                if (pt.isEmpty()) continue;

                QDate peForecast = pt["forecastEnd"].toDate();
                QDate peTarget   = pt["endDate"].toDate();
                QDate pe = peForecast.isValid() ? peForecast : peTarget;

                if (pe.isValid() && (!maxPredEnd.isValid() || pe > maxPredEnd))
                    maxPredEnd = pe;
            }
            controller->get_projectData()->set_LinkedPredecessorEndDate(maxPredEnd);

            // Сохраняем ЛГ-файл, чтобы границы были актуальны и при следующем открытии.
            controller->saveProject();
        }
    }

    engine->rootContext()->setContextProperty("projectController", controller);
    engine->rootContext()->setContextProperty("sessionManager", this);

    const QUrl url(QStringLiteral("qrc:/qml/main.qml"));
    engine->load(url);

    if (engine->rootObjects().isEmpty())
    {
        qWarning() << "SessionManager: failed to load QML for local graph";
        delete controller;
        delete engine;
        return false;
    }

    QObject* window = engine->rootObjects().first();

    LocalSession session;
    session.engine = engine;
    session.controller = controller;
    session.window = window;
    session.taskId = taskId;

    m_localSessions.insert(taskId, session);

    // При сохранении ЛГ уведомляем главное окно
    connect(controller, &ProjectController::projectSaved, this, [this, taskId, controller]() {
        QString path = controller->get_projectData()->get_filePath();
        emit localGraphSaved(taskId, path);
    });

    QQuickWindow* qw = qobject_cast<QQuickWindow*>(window);
    if (qw) { qw->raise(); qw->requestActivate(); }

    return true;
}

void SessionManager::closeLocalGraphSession(const QString& taskId)
{
    if (!m_localSessions.contains(taskId)) return;

    LocalSession session = m_localSessions.take(taskId);
    if (session.window)
        session.window->deleteLater();
}

void SessionManager::notifyWindowClosed(const QString& taskId)
{
    if (!m_localSessions.contains(taskId)) return;

    LocalSession session = m_localSessions.take(taskId);

    // Обновляем прогноз в мастер-графике по актуальному файлу ЛГ
    if (m_masterController && !taskId.isEmpty())
    {
        QString path = m_masterController->get_localGraphController()->getLocalGraphPath(taskId);
        if (!path.isEmpty() && QFile::exists(path))
        {
            m_masterController->get_localGraphController()->refreshLocalGraphForecast(taskId, path, true);
            emit localGraphSaved(taskId, path);
        }
    }

    if (session.window) session.window->deleteLater();
    if (session.controller) session.controller->deleteLater();
    if (session.engine) session.engine->deleteLater();

    emit localGraphSessionClosed(taskId);
}

void SettingsManager::setHolidaysFromJson(const QVariantList& holidays)
{
    m_holidays.clear();
    for (const QVariant& item : holidays)
    {
        QVariantMap h = item.toMap();
        Holiday hol;
        hol.name = h.value("name").toString();
        QDate start = QDate::fromString(h.value("start_date").toString(), Qt::ISODate);
        QDate end = QDate::fromString(h.value("end_date").toString(), Qt::ISODate);
        if (!start.isValid()) continue;
        hol.startDay = start.day();
        hol.startMonth = start.month();
        hol.startYear = start.year();
        hol.isYearly = h.value("is_yearly").toBool();
        if (end.isValid()) {
            hol.endDay = end.day();
            hol.endMonth = end.month();
            hol.endYear = end.year();
        } else {
            hol.endDay = hol.startDay;
            hol.endMonth = hol.startMonth;
            hol.endYear = hol.startYear;
        }
        m_holidays.append(hol);
    }
    emit holidaysChanged();
}
