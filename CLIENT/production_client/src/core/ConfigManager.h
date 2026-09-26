#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QFont>

class ConfigManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY themeChanged)
    Q_PROPERTY(QString colorScheme READ colorScheme WRITE setColorScheme NOTIFY colorSchemeChanged)
    Q_PROPERTY(QString lightScheme READ lightScheme WRITE setLightScheme NOTIFY lightSchemeChanged)
    Q_PROPERTY(QString darkScheme READ darkScheme WRITE setDarkScheme NOTIFY darkSchemeChanged)
    Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize NOTIFY fontSizeChanged)

public:
    static ConfigManager* instance();

    QString theme() const { return m_theme; }
    QString colorScheme() const { return m_colorScheme; }
    QString lightScheme() const { return m_lightScheme; }
    QString darkScheme() const { return m_darkScheme; }
    int fontSize() const { return m_fontSize; }

    Q_INVOKABLE void setTheme(const QString& theme);
    Q_INVOKABLE void setColorScheme(const QString& colorScheme);
    Q_INVOKABLE void setLightScheme(const QString& scheme);
    Q_INVOKABLE void setDarkScheme(const QString& scheme);
    Q_INVOKABLE void setFontSize(int fontSize);

signals:
    void themeChanged(const QString& theme);
    void colorSchemeChanged(const QString& colorScheme);
    void lightSchemeChanged(const QString& scheme);
    void darkSchemeChanged(const QString& scheme);
    void fontSizeChanged(int fontSize);

private:
    explicit ConfigManager(QObject* parent = nullptr);
    ~ConfigManager();

    void load();
    void save();
    QString getConfigPath() const;
    QJsonObject readConfigFile();
    void writeConfigFile(const QJsonObject& config);

    QString m_theme;
    QString m_colorScheme;
    QString m_lightScheme;
    QString m_darkScheme;
    int m_fontSize;
    bool m_isLoaded;
};

#endif // CONFIGMANAGER_H
