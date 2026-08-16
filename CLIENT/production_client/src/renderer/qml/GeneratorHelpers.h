#ifndef GENERATORHELPERS_H
#define GENERATORHELPERS_H

#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>

void parseChartData(const QJsonObject& spec, QJsonArray& values, QStringList& labels);
QString generateColorArray(int size);

#endif // GENERATORHELPERS_H
