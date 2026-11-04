#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QSettings>
#include <QVariantList>
#include <QVariantMap>
#include <QDate>
#include "GlobalDefines.h"

class SettingsManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString resourcesPath READ resourcesPath WRITE setResourcesPath NOTIFY resourcesPathChanged)
    Q_PROPERTY(QString holidaysPath READ holidaysPath WRITE setHolidaysPath NOTIFY holidaysPathChanged)
    Q_PROPERTY(GanttDefines::ZoomLevel zoomLevel READ zoomLevel WRITE setZoomLevel NOTIFY zoomLevelChanged)
    Q_PROPERTY(bool tracingEnabled READ tracingEnabled WRITE setTracingEnabled NOTIFY tracingEnabledChanged)
    Q_PROPERTY(bool editingLocked READ editingLocked WRITE setEditingLocked NOTIFY editingLockedChanged)
    Q_PROPERTY(GanttDefines::ViewMode viewMode READ viewMode WRITE setViewMode NOTIFY viewModeChanged)
    Q_PROPERTY(bool showHolidays READ showHolidays WRITE setShowHolidays NOTIFY showHolidaysChanged)

public:
    explicit SettingsManager(QObject *parent = nullptr);

    QString resourcesPath() const;
    void setResourcesPath(const QString& path);

    QString holidaysPath() const;
    void setHolidaysPath(const QString& path);

    GanttDefines::ZoomLevel zoomLevel() const;
    void setZoomLevel(GanttDefines::ZoomLevel level);
    Q_INVOKABLE void setZoomLevel(int level);

    bool tracingEnabled() const;
    void setTracingEnabled(bool enabled);

    bool editingLocked() const;
    void setEditingLocked(bool locked);

    GanttDefines::ViewMode viewMode() const;
    void setViewMode(GanttDefines::ViewMode mode);
    Q_INVOKABLE void setViewMode(int mode);

    bool showHolidays() const;
    void setShowHolidays(bool show);

    Q_INVOKABLE bool isHoliday(const QDate& date) const;
    Q_INVOKABLE QVariantMap getHolidayInfo(const QDate& date) const;
    Q_INVOKABLE QString holidaysTooltip(const QDate& date) const;
    Q_INVOKABLE QVariantList holidays() const;
    Q_INVOKABLE void reloadHolidays();

    Q_INVOKABLE void saveSettings();
    Q_INVOKABLE void loadSettings();
    Q_INVOKABLE void setHolidaysFromJson(const QVariantList& holidays);

signals:
    void resourcesPathChanged();
    void holidaysPathChanged();
    void zoomLevelChanged();
    void tracingEnabledChanged();
    void editingLockedChanged();
    void viewModeChanged();
    void showHolidaysChanged();
    void holidaysChanged();

private:
    struct Holiday
    {
        QString name;
        int     startDay = 0;
        int     startMonth = 0;
        int     startYear = 0;
        int     endDay = 0;
        int     endMonth = 0;
        int     endYear = 0;
        bool    isYearly = false;
    };

    QSettings* m_settings;
    QString m_resourcesPath;
    QString m_holidaysPath;
    GanttDefines::ZoomLevel m_zoomLevel;
    bool m_tracingEnabled;
    bool m_editingLocked;
    GanttDefines::ViewMode m_viewMode;
    bool m_showHolidays;

    QList<Holiday> m_holidays;

    void parseHolidaysFile(const QString& filePath);
    QDate parseDate(const QString& s, bool& ok, bool& yearly) const;
    bool holidayMatchesDate(const Holiday& h, const QDate& date) const;
};

#endif
