#include "ConfigManager.h"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QDebug>
#include <QStandardPaths>

ConfigManager* ConfigManager::instance()
{
    static ConfigManager* instance = nullptr;
    if (!instance) {
        instance = new ConfigManager();
    }
    return instance;
}

ConfigManager::ConfigManager(QObject* parent)
    : QObject(parent)
    , m_theme("dark")
    , m_colorScheme("blue")
    , m_lightScheme("blue")
    , m_darkScheme("blue")
    , m_fontSize(14)
    , m_isLoaded(false)
{
    load();
}

ConfigManager::~ConfigManager()
{
    save();
}

QString ConfigManager::getConfigPath() const
{
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    if (configDir.isEmpty()) {
        configDir = QDir::homePath() + "/.config";
    }
    QString appDir = configDir + "/EngineeringRE";
    
    QDir dir;
    if (!dir.exists(appDir)) {
        dir.mkpath(appDir);
    }
    
    return appDir + "/config.json";
}

QJsonObject ConfigManager::readConfigFile()
{
    QString path = getConfigPath();
    QFile file(path);
    
    if (!file.exists()) {
        qDebug() << "ConfigManager: Config file not found, using defaults";
        return QJsonObject();
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "ConfigManager: Failed to open config file for reading:" << path;
        return QJsonObject();
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qDebug() << "ConfigManager: Failed to parse config file:" << error.errorString();
        return QJsonObject();
    }
    
    if (!doc.isObject()) {
        qDebug() << "ConfigManager: Config file is not a JSON object";
        return QJsonObject();
    }
    
    return doc.object();
}

void ConfigManager::writeConfigFile(const QJsonObject& config)
{
    QString path = getConfigPath();
    QFile file(path);
    
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "ConfigManager: Failed to open config file for writing:" << path;
        return;
    }
    
    QJsonDocument doc(config);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    qDebug() << "ConfigManager: Config saved to" << path;
}

void ConfigManager::load()
{
    if (m_isLoaded) {
        return;
    }
    
    QJsonObject config = readConfigFile();
    
    if (config.isEmpty()) {
        qDebug() << "ConfigManager: Using default settings";
        m_theme = "dark";
        m_colorScheme = "blue";
        m_lightScheme = "blue";
        m_darkScheme = "blue";
        m_fontSize = 14;
    } else {
        m_theme = config.value("theme").toString("dark");
        m_colorScheme = config.value("colorScheme").toString("blue");
        m_lightScheme = config.value("lightScheme").toString("blue");
        m_darkScheme = config.value("darkScheme").toString("blue");
        m_fontSize = config.value("fontSize").toInt(14);
        
        if (m_fontSize < 10 || m_fontSize > 20) {
            m_fontSize = 14;
        }
    }
    
    m_isLoaded = true;
    qDebug() << "ConfigManager: Loaded settings - theme:" << m_theme 
             << "colorScheme:" << m_colorScheme
             << "lightScheme:" << m_lightScheme
             << "darkScheme:" << m_darkScheme
             << "fontSize:" << m_fontSize;
}

void ConfigManager::save()
{
    QJsonObject config;
    config["theme"] = m_theme;
    config["colorScheme"] = m_colorScheme;
    config["lightScheme"] = m_lightScheme;
    config["darkScheme"] = m_darkScheme;
    config["fontSize"] = m_fontSize;
    
    writeConfigFile(config);
}

void ConfigManager::setTheme(const QString& theme)
{
    if (m_theme == theme) {
        return;
    }
    
    m_theme = theme;
    save();
    emit themeChanged(theme);
    qDebug() << "ConfigManager: Theme changed to" << theme;
}

void ConfigManager::setColorScheme(const QString& colorScheme)
{
    if (m_colorScheme == colorScheme) {
        return;
    }
    
    m_colorScheme = colorScheme;
    save();
    emit colorSchemeChanged(colorScheme);
    qDebug() << "ConfigManager: Color scheme changed to" << colorScheme;
}

void ConfigManager::setLightScheme(const QString& scheme)
{
    if (m_lightScheme == scheme) return;
    m_lightScheme = scheme;
    save();
    emit lightSchemeChanged(scheme);
    qDebug() << "ConfigManager: Light scheme changed to" << scheme;
}

void ConfigManager::setDarkScheme(const QString& scheme)
{
    if (m_darkScheme == scheme) return;
    m_darkScheme = scheme;
    save();
    emit darkSchemeChanged(scheme);
    qDebug() << "ConfigManager: Dark scheme changed to" << scheme;
}

void ConfigManager::setFontSize(int fontSize)
{
    if (fontSize < 10) {
        fontSize = 10;
    }
    if (fontSize > 20) {
        fontSize = 20;
    }
    
    if (m_fontSize == fontSize) {
        return;
    }
    
    m_fontSize = fontSize;
    save();
    emit fontSizeChanged(fontSize);
    qDebug() << "ConfigManager: Font size changed to" << fontSize;
}
