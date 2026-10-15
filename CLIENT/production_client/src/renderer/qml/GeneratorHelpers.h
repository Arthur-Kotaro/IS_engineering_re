#ifndef GENERATORHELPERS_H
#define GENERATORHELPERS_H

#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QStringList>

void parseChartData(const QJsonObject &spec, QJsonArray &values, QStringList &labels);
QString generateColorArray(int size);

#endif // GENERATORHELPERS_H
