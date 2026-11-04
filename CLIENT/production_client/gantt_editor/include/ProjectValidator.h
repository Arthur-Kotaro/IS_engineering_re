#ifndef PROJECTVALIDATOR_H
#define PROJECTVALIDATOR_H

#include <QObject>
#include <QVariantMap>
#include <QStringList>

class ProjectValidator : public QObject
{
    Q_OBJECT
public:
    explicit ProjectValidator(QObject *parent = nullptr);

    // Возвращает true, если валидация пройдена. Ошибки накапливаются в errors().
    bool validate(const QVariantMap& json);

    QStringList errors() const { return m_errors; }

private:
    QStringList m_errors;
    
    void checkRequiredFields(const QVariantMap& json);
    void checkGroups(const QVariantMap& json);
    void checkTasks(const QVariantMap& json);
    void checkDependencies(const QVariantMap& json);
    void checkMilestones(const QVariantMap& json);
};

#endif
