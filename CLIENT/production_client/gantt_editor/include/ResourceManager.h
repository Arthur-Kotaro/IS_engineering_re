#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <QObject>
#include <QVariantMap>
#include <QList>

class ResourceManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString resourcesPath READ resourcesPath WRITE setResourcesPath NOTIFY resourcesPathChanged)

public:
    explicit ResourceManager(QObject *parent = nullptr);

    QString resourcesPath() const;
    void setResourcesPath(const QString& path);

    Q_INVOKABLE QVariantList loadTypologies();
    Q_INVOKABLE QVariantList loadTaskGroups();

    bool saveProjectToFile(const QVariantMap& projectData, const QString& filePath);
    QVariantMap loadProjectFromFile(const QString& filePath);

signals:
    void resourcesPathChanged();
    void errorOccurred(const QString& message);

private:
    QString m_resourcesPath;
    QString getDefaultResourcesPath() const;
    QVariantMap loadJsonFile(const QString& fileName);
    bool saveJsonFile(const QString& fileName, const QVariantMap& data);
};

#endif
