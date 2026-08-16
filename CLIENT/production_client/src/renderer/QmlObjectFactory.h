#ifndef QMLOBJECTFACTORY_H
#define QMLOBJECTFACTORY_H

#include <QObject>
#include <QJsonObject>
#include <QMap>
#include <functional>

class QQmlEngine;
class QQuickItem;

class QmlObjectFactory : public QObject
{
    Q_OBJECT

public:
    explicit QmlObjectFactory(QQmlEngine* engine, QObject* parent = nullptr);

    QObject* create(const QString& type, const QJsonObject& spec, QQuickItem* parent);

private:
    void registerBuiltInTypes();

    QString generateQmlCode(const QString& type, const QJsonObject& spec);
    QString applyProperties(const QString& qmlCode, const QJsonObject& spec);

    QQmlEngine* m_engine;
    QMap<QString, std::function<QString(const QJsonObject&)>> m_qmlGenerators;
};

#endif // QMLOBJECTFACTORY_H
