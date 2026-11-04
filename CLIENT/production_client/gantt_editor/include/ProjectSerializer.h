#ifndef PROJECTSERIALIZER_H
#define PROJECTSERIALIZER_H

#include <QObject>
#include <QVariantMap>
#include <QStringList>

class ProjectData;

class ProjectSerializer : public QObject
{
    Q_OBJECT
public:
    explicit ProjectSerializer(QObject *parent = nullptr);

    // Сериализация в JSON (QVariantMap)
    QVariantMap toJson(const ProjectData* project) const;

    // Десериализация. Возвращает true при успехе.
    bool fromJson(ProjectData* project, const QVariantMap& json, QStringList& validationErrors);
};

#endif
