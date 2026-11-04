#include "ResourceManager.h"
#include <QFile>
#include <QSaveFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QStandardPaths>

ResourceManager::ResourceManager(QObject *parent) : QObject(parent)
{
    m_resourcesPath = getDefaultResourcesPath();
}

QString ResourceManager::resourcesPath() const { return m_resourcesPath; }

void ResourceManager::setResourcesPath(const QString& path)
{
    if (m_resourcesPath != path)
    {
        m_resourcesPath = path;
        emit resourcesPathChanged();
    }
}

QString ResourceManager::getDefaultResourcesPath() const
{
    return QCoreApplication::applicationDirPath() + "/";
}

QVariantMap ResourceManager::loadJsonFile(const QString& fileName)
{
    QFile qrcFile(":/" + fileName);
    if (qrcFile.open(QIODevice::ReadOnly))
    {
        QByteArray data = qrcFile.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isNull())
        {
            qDebug() << "Loaded resource from qrc:" << fileName;
            return doc.object().toVariantMap();
        }
    }

    QString fullPath = m_resourcesPath + fileName;
    QFile file(fullPath);
    if (!file.open(QIODevice::ReadOnly))
    {
        emit errorOccurred("Не удалось открыть файл: " + fullPath);
        return QVariantMap();
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull())
    {
        emit errorOccurred("Ошибка парсинга JSON: " + fullPath);
        return QVariantMap();
    }

    qDebug() << "Loaded resource from file:" << fileName;
    return doc.object().toVariantMap();
}

bool ResourceManager::saveJsonFile(const QString& fileName, const QVariantMap& data)
{
    QString fullPath = fileName;

    QFileInfo fileInfo(fullPath);
    QDir dir = fileInfo.absoluteDir();

    if (!dir.exists())
    {
        if (!dir.mkpath("."))
        {
            emit errorOccurred("Не удалось создать директорию: " + dir.absolutePath());
            return false;
        }
    }

    // 1. Бэкап: если файл существует, копируем его в .bak (перезаписывая старый)
    if (QFile::exists(fullPath))
    {
        QString backupPath = fullPath + ".bak";
        QFile::remove(backupPath); // Удаляем старый бэкап, чтобы copy не упал
        QFile::copy(fullPath, backupPath);
    }

    // 2. Атомарная запись через QSaveFile
    QSaveFile file(fullPath);
    if (!file.open(QIODevice::WriteOnly))
    {
        emit errorOccurred("Не удалось открыть файл для записи: " + fullPath);
        return false;
    }

    QJsonDocument doc = QJsonDocument::fromVariant(data);
    file.write(doc.toJson());

    if (!file.commit())
    {
        emit errorOccurred("Ошибка записи файла: " + fullPath);
        return false;
    }

    qDebug() << "Project saved to:" << fullPath;
    return true;
}

QVariantList ResourceManager::loadTypologies()
{
    QVariantMap data = loadJsonFile("default_typologies.json");
    QVariantList result = data["typologies"].toList();
    qDebug() << "Typologies loaded, count:" << result.size();
    return result;
}

QVariantList ResourceManager::loadTaskGroups()
{
    QVariantMap data = loadJsonFile("default_tasks.json");
    QVariantList result = data["task_groups"].toList();
    qDebug() << "Task groups loaded, count:" << result.size();
    return result;
}

bool ResourceManager::saveProjectToFile(const QVariantMap& projectData, const QString& filePath)
{
    return saveJsonFile(filePath, projectData);
}

QVariantMap ResourceManager::loadProjectFromFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        emit errorOccurred("Не удалось открыть файл проекта: " + filePath);
        return QVariantMap();
    }

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull())
    {
        emit errorOccurred("Ошибка парсинга файла проекта: " + filePath);
        return QVariantMap();
    }

    QVariantMap projectData = doc.object().toVariantMap();
    qDebug() << "Project loaded from:" << filePath
             << "groups:" << projectData["groups"].toList().size()
             << "tasks:" << projectData["tasks"].toList().size();
    return projectData;
}
