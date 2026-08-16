#include "GeneratorHelpers.h"
#include <QDebug>

void parseChartData(const QJsonObject& spec, QJsonArray& values, QStringList& labels)
{
    values = QJsonArray();
    labels = QStringList();

    if (!spec.contains("data") || !spec["data"].isObject()) {
        values = {30, 25, 20, 15, 10};
        labels = {"Категория 1", "Категория 2", "Категория 3", "Категория 4", "Категория 5"};
        return;
    }

    QJsonObject dataObj = spec["data"].toObject();

    if (dataObj.contains("Marked_values") && dataObj["Marked_values"].isArray()) {
        QJsonArray markedValues = dataObj["Marked_values"].toArray();
        for (const QJsonValue& item : markedValues) {
            if (item.isObject()) {
                QJsonObject obj = item.toObject();
                if (obj.contains("value")) {
                    values.append(obj["value"].toDouble());
                    if (obj.contains("label")) {
                        labels << obj["label"].toString();
                    } else {
                        labels << QString("Значение %1").arg(values.size());
                    }
                }
            }
        }
        if (!values.isEmpty()) {
            return;
        }
    }

    if (dataObj.contains("values") && dataObj["values"].isArray()) {
        QJsonArray valuesArray = dataObj["values"].toArray();
        for (const QJsonValue& val : valuesArray) {
            if (val.isDouble()) {
                values.append(val.toDouble());
                labels << QString("Значение %1").arg(values.size());
            }
        }
        if (!values.isEmpty()) {
            return;
        }
    }

    values = {30, 25, 20, 15, 10};
    labels = {"Категория 1", "Категория 2", "Категория 3", "Категория 4", "Категория 5"};
}

QString generateColorArray(int size)
{
    QStringList colors = {
        "#ff6b6b", "#feca57", "#48dbfb", "#ff9ff3", "#54a0ff",
        "#1dd1a1", "#f368e0", "#00d2d3", "#ff9f43", "#a29bfe"
    };
    
    QString result = "[";
    for (int i = 0; i < size; ++i) {
        if (i > 0) result += ", ";
        result += "\"" + colors[i % colors.size()] + "\"";
    }
    result += "]";
    return result;
}
