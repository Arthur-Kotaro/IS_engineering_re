#include "SettingsManager.h"
#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

SettingsManager::SettingsManager(QObject *parent): QObject(parent)
    , m_zoomLevel(GanttDefines::ZoomLevel::Daily)
    , m_tracingEnabled(false)
    , m_editingLocked(false)
    , m_viewMode(GanttDefines::ViewMode::Target)
    , m_showHolidays(false)
{
    m_settings = new QSettings(QCoreApplication::organizationName(), QCoreApplication::applicationName(), this);
    loadSettings();

    if (m_holidaysPath.isEmpty())
        m_holidaysPath = QCoreApplication::applicationDirPath() + "/default_holidays.json";

    reloadHolidays();

    qDebug() << "SettingsManager initialized, zoomLevel:" << static_cast<int>(m_zoomLevel)
             << "viewMode:" << static_cast<int>(m_viewMode)
             << "holidaysPath:" << m_holidaysPath
             << "holidays count:" << m_holidays.size();
}

QString SettingsManager::resourcesPath() const { return m_resourcesPath; }

void SettingsManager::setResourcesPath(const QString& path) {
    if (m_resourcesPath != path) {
        m_resourcesPath = path;
        emit resourcesPathChanged();
        saveSettings();
    }
}

QString SettingsManager::holidaysPath() const { return m_holidaysPath; }

void SettingsManager::setHolidaysPath(const QString& path) {
    if (m_holidaysPath != path) {
        m_holidaysPath = path;
        emit holidaysPathChanged();
        saveSettings();
        reloadHolidays();
    }
}

GanttDefines::ZoomLevel SettingsManager::zoomLevel() const { return m_zoomLevel; }

void SettingsManager::setZoomLevel(GanttDefines::ZoomLevel level) {
    if (m_zoomLevel != level) {
        m_zoomLevel = level;
        emit zoomLevelChanged();
        saveSettings();
    }
}

void SettingsManager::setZoomLevel(int level)
{
    if (level >= 0 && level <= 1) setZoomLevel(static_cast<GanttDefines::ZoomLevel>(level));
}

bool SettingsManager::tracingEnabled() const { return m_tracingEnabled; }

void SettingsManager::setTracingEnabled(bool enabled) {
    if (m_tracingEnabled != enabled) {
        m_tracingEnabled = enabled;
        emit tracingEnabledChanged();
        saveSettings();
    }
}

bool SettingsManager::editingLocked() const { return m_editingLocked; }

void SettingsManager::setEditingLocked(bool locked) {
    if (m_editingLocked != locked) {
        m_editingLocked = locked;
        emit editingLockedChanged();
        saveSettings();
    }
}

GanttDefines::ViewMode SettingsManager::viewMode() const { return m_viewMode; }

void SettingsManager::setViewMode(GanttDefines::ViewMode mode) {
    if (m_viewMode != mode) {
        m_viewMode = mode;
        emit viewModeChanged();
        saveSettings();
    }
}

void SettingsManager::setViewMode(int mode) {
    if (mode >= 0 && mode <= 2) {
        setViewMode(static_cast<GanttDefines::ViewMode>(mode));
    }
}

bool SettingsManager::showHolidays() const { return m_showHolidays; }

void SettingsManager::setShowHolidays(bool show) {
    if (m_showHolidays != show) {
        m_showHolidays = show;
        emit showHolidaysChanged();
        saveSettings();
    }
}

QDate SettingsManager::parseDate(const QString& s, bool& ok, bool& yearly) const
{
    ok = false;
    yearly = false;

    QStringList parts = s.split(".");
    if (parts.length() == 2)
    {
        yearly = true;
        int d = parts[0].toInt();
        int m = parts[1].toInt();
        if (d > 0 && m > 0)
        {
            ok = true;
            return QDate(2000, m, d);
        }
    }
    else if (parts.length() == 3)
    {
        yearly = false;
        QDate date = QDate::fromString(s, "dd.MM.yyyy");
        if (date.isValid())
        {
            ok = true;
            return date;
        }
    }
    return QDate();
}

bool SettingsManager::holidayMatchesDate(const Holiday& h, const QDate& date) const
{
    if (!date.isValid()) return false;

    if (h.isYearly)
    {
        QDate start(date.year(), h.startMonth, h.startDay);
        QDate end(date.year(), h.endMonth, h.endDay);
        if (end >= start)
        {
            if (date >= start && date <= end) return true;
        }
        else
        {
            if (date >= start || date <= end) return true;
        }
    }
    else
    {
        QDate start(h.startYear, h.startMonth, h.startDay);
        QDate end = (h.endYear > 0)
                  ? QDate(h.endYear, h.endMonth, h.endDay)
                  : start;
        if (date >= start && date <= end) return true;
    }
    return false;
}

void SettingsManager::parseHolidaysFile(const QString& filePath)
{
    m_holidays.clear();

    QByteArray data;
    QFile file(filePath);
    if (file.exists() && file.open(QIODevice::ReadOnly))
    {
        data = file.readAll();
        file.close();
    }
    else
    {
        QFile qrcFile(":/default_holidays.json");
        if (qrcFile.open(QIODevice::ReadOnly))
        {
            data = qrcFile.readAll();
            qrcFile.close();
        }
    }

    if (data.isEmpty())
    {
        qWarning() << "SettingsManager: no holidays data available";
        emit holidaysChanged();
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull())
    {
        qWarning() << "SettingsManager: invalid JSON in holidays";
        emit holidaysChanged();
        return;
    }

    QJsonArray arr = doc.object().value("holidays").toArray();
    for (const auto& item : arr)
    {
        QJsonObject obj = item.toObject();
        Holiday h;
        h.name = obj.value("holiday_name").toString();
        QString startStr = obj.value("holiday_start_date").toString();
        QString endStr   = obj.value("holiday_end_date").toString();

        bool okS = false, yearlyS = false;
        QDate s = parseDate(startStr, okS, yearlyS);
        if (!okS) continue;

        h.startDay = s.day();
        h.startMonth = s.month();
        h.startYear = yearlyS ? 0 : s.year();
        h.isYearly = yearlyS;

        if (!endStr.isEmpty())
        {
            bool okE = false, yearlyE = false;
            QDate e = parseDate(endStr, okE, yearlyE);
            if (okE)
            {
                h.endDay = e.day();
                h.endMonth = e.month();
                h.endYear = yearlyE ? 0 : e.year();
            }
            else
            {
                h.endDay = h.startDay;
                h.endMonth = h.startMonth;
                h.endYear = h.startYear;
            }
        }
        else
        {
            h.endDay = h.startDay;
            h.endMonth = h.startMonth;
            h.endYear = h.startYear;
        }

        m_holidays.append(h);
    }

    qDebug() << "SettingsManager: loaded" << m_holidays.size() << "holidays from" << filePath;
    emit holidaysChanged();
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
    qDebug() << "SettingsManager: setHolidaysFromJson ->" << m_holidays.size() << "holidays";
    emit holidaysChanged();
}

void SettingsManager::reloadHolidays()
{
    parseHolidaysFile(m_holidaysPath);
}

QVariantList SettingsManager::holidays() const
{
    QVariantList result;
    for (const auto& h : m_holidays)
    {
        QVariantMap m;
        m["name"] = h.name;
        m["isYearly"] = h.isYearly;
        result.append(m);
    }
    return result;
}

bool SettingsManager::isHoliday(const QDate& date) const
{
    for (const auto& h : m_holidays)
    {
        if (holidayMatchesDate(h, date)) return true;
    }
    return false;
}

QVariantMap SettingsManager::getHolidayInfo(const QDate& date) const
{
    QVariantMap result;
    for (const auto& h : m_holidays)
    {
        if (holidayMatchesDate(h, date))
        {
            result["name"] = h.name;
            result["isYearly"] = h.isYearly;

            QDate start;
            QDate end;
            if (h.isYearly)
            {
                start = QDate(date.year(), h.startMonth, h.startDay);
                end   = QDate(date.year(), h.endMonth, h.endDay);
            }
            else
            {
                start = QDate(h.startYear, h.startMonth, h.startDay);
                end   = (h.endYear > 0) ? QDate(h.endYear, h.endMonth, h.endDay) : start;
            }
            result["startDate"] = start;
            result["endDate"] = end;
            return result;
        }
    }
    return result;
}

QString SettingsManager::holidaysTooltip(const QDate& date) const
{
    QVariantMap info = getHolidayInfo(date);
    if (info.isEmpty()) return "";

    QString name = info["name"].toString();
    QDate start = info["startDate"].toDate();
    QDate end = info["endDate"].toDate();

    QString dateStr;
    if (start.isValid() && end.isValid() && start != end)
    {
        dateStr = start.toString("dd.MM.yyyy") + " — " + end.toString("dd.MM.yyyy");
    }
    else if (start.isValid())
    {
        dateStr = start.toString("dd.MM.yyyy");
    }

    if (dateStr.isEmpty()) return name;
    return name + "\n" + dateStr;
}

void SettingsManager::saveSettings()
{
    m_settings->setValue("resourcesPath", m_resourcesPath);
    m_settings->setValue("holidaysPath", m_holidaysPath);
    m_settings->setValue("zoomLevel", static_cast<int>(m_zoomLevel));
    m_settings->setValue("tracingEnabled", m_tracingEnabled);
    m_settings->setValue("editingLocked", m_editingLocked);
    m_settings->setValue("viewMode", static_cast<int>(m_viewMode));
    m_settings->setValue("showHolidays", m_showHolidays);
    m_settings->sync();
}

void SettingsManager::loadSettings()
{
    m_resourcesPath = m_settings->value("resourcesPath",
        QCoreApplication::applicationDirPath() + "/").toString();
    m_holidaysPath = m_settings->value("holidaysPath",
        QCoreApplication::applicationDirPath() + "/default_holidays.json").toString();

        m_zoomLevel = static_cast<GanttDefines::ZoomLevel>(
            m_settings->value("zoomLevel", 1).toInt());

        int zoomRaw = static_cast<int>(m_zoomLevel);
        if (zoomRaw < 0 || zoomRaw > 1)
        {
            m_zoomLevel = GanttDefines::ZoomLevel::Daily;
        }
    m_tracingEnabled = m_settings->value("tracingEnabled", false).toBool();
    m_editingLocked = m_settings->value("editingLocked", false).toBool();
    m_viewMode = static_cast<GanttDefines::ViewMode>(
        m_settings->value("viewMode", 0).toInt());
    m_showHolidays = m_settings->value("showHolidays", false).toBool();
}
