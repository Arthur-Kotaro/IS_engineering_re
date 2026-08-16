#include "GeneratorHelpers.h"
#include <QString>
#include <QJsonObject>
#include <QJsonArray>

QString generatePieChart(const QJsonObject& spec)
{
    QString title = spec["title"].toString("Диаграмма");
    int preferredHeight = spec["height"].toInt(350);
    if (preferredHeight <= 0) preferredHeight = 350;

    QJsonArray values;
    QStringList labels;
    parseChartData(spec, values, labels);

    QStringList colors = {
        "#ff6b6b", "#feca57", "#48dbfb", "#ff9ff3", "#54a0ff",
        "#1dd1a1", "#f368e0", "#00d2d3", "#ff9f43", "#a29bfe"
    };

    QString valuesStr = "[";
    QString labelsStr = "[";
    QString colorsStr = "[";
    for (int i = 0; i < values.size(); ++i) {
        if (i > 0) {
            valuesStr += ", ";
            labelsStr += ", ";
            colorsStr += ", ";
        }
        double val = values[i].toDouble();
        valuesStr += QString::number(val);
        labelsStr += "\"" + labels[i] + "\"";
        colorsStr += "\"" + colors[i % colors.size()] + "\"";
    }
    valuesStr += "]";
    labelsStr += "]";
    colorsStr += "]";

    // Простая версия с одним Canvas и фиксированными данными для теста
    return QString(
        "Rectangle {\n"
        "    color: \"transparent\"\n"
        "    Layout.fillWidth: true\n"
        "    Layout.preferredHeight: %1\n"
        "    ColumnLayout {\n"
        "        anchors.fill: parent\n"
        "        Text {\n"
        "            text: \"%2\"\n"
        "            color: \"#aaa\"\n"
        "            font.pixelSize: 14\n"
        "            font.bold: true\n"
        "            Layout.alignment: Qt.AlignHCenter\n"
        "            Layout.preferredHeight: implicitHeight\n"
        "        }\n"
        "        Rectangle {\n"
        "            Layout.fillWidth: true\n"
        "            Layout.fillHeight: true\n"
        "            color: \"#1a1a1a\"\n"
        "            border.color: \"#444\"\n"
        "            border.width: 1\n"
        "            radius: 4\n"
        "            Canvas {\n"
        "                id: pieCanvas\n"
        "                anchors.fill: parent\n"
        "                anchors.margins: 10\n"
        "                property var data: %3\n"
        "                property var labels: %4\n"
        "                property var colors: %5\n"
        "                property int hoveredIndex: -1\n"
        "                \n"
        "                onPaint: {\n"
        "                    console.log(\"PieChart: onPaint called, data=\", data, \"length=\", data ? data.length : 0)\n"
        "                    if (!data || data.length === 0) {\n"
        "                        console.log(\"PieChart: no data, drawing placeholder\")\n"
        "                        var ctx = getContext(\"2d\");\n"
        "                        ctx.fillStyle = \"#444\";\n"
        "                        ctx.fillRect(0, 0, width, height);\n"
        "                        ctx.fillStyle = \"#888\";\n"
        "                        ctx.font = \"20px sans-serif\";\n"
        "                        ctx.textAlign = \"center\";\n"
        "                        ctx.fillText(\"Нет данных\", width/2, height/2);\n"
        "                        return;\n"
        "                    }\n"
        "                    var ctx = getContext(\"2d\");\n"
        "                    var cx = width / 2;\n"
        "                    var cy = height / 2;\n"
        "                    var r = Math.min(width, height) / 2 - 20;\n"
        "                    if (r < 10) r = 10;\n"
        "                    var total = 0;\n"
        "                    for (var i = 0; i < data.length; i++) {\n"
        "                        total += data[i];\n"
        "                    }\n"
        "                    if (total === 0) total = 1;\n"
        "                    var startAngle = 0;\n"
        "                    for (var i = 0; i < data.length; i++) {\n"
        "                        var angle = (data[i] / total) * 2 * Math.PI;\n"
        "                        var isHovered = (i === hoveredIndex);\n"
        "                        var radius = isHovered ? r * 1.08 : r;\n"
        "                        var offsetX = 0;\n"
        "                        var offsetY = 0;\n"
        "                        if (isHovered) {\n"
        "                            var midAngle = startAngle + angle / 2;\n"
        "                            offsetX = Math.cos(midAngle) * r * 0.08;\n"
        "                            offsetY = Math.sin(midAngle) * r * 0.08;\n"
        "                        }\n"
        "                        ctx.beginPath();\n"
        "                        ctx.moveTo(cx + offsetX, cy + offsetY);\n"
        "                        ctx.arc(cx + offsetX, cy + offsetY, radius, startAngle, startAngle + angle);\n"
        "                        ctx.closePath();\n"
        "                        ctx.fillStyle = colors[i % colors.length];\n"
        "                        ctx.fill();\n"
        "                        ctx.strokeStyle = \"#333\";\n"
        "                        ctx.lineWidth = isHovered ? 3 : 2;\n"
        "                        ctx.stroke();\n"
        "                        startAngle += angle;\n"
        "                    }\n"
        "                }\n"
        "                \n"
        "                Component.onCompleted: {\n"
        "                    console.log(\"PieChart: Component.onCompleted, data=\", data)\n"
        "                    requestPaint()\n"
        "                }\n"
        "                onDataChanged: {\n"
        "                    console.log(\"PieChart: onDataChanged, data=\", data)\n"
        "                    requestPaint()\n"
        "                }\n"
        "                onWidthChanged: {\n"
        "                    console.log(\"PieChart: onWidthChanged\", width)\n"
        "                    requestPaint()\n"
        "                }\n"
        "                onHeightChanged: {\n"
        "                    console.log(\"PieChart: onHeightChanged\", height)\n"
        "                    requestPaint()\n"
        "                }\n"
        "            }\n"
        "        }\n"
        "        Flow {\n"
        "            Layout.fillWidth: true\n"
        "            Layout.preferredHeight: implicitHeight\n"
        "            spacing: 20\n"
        "            padding: 8\n"
        "            Repeater {\n"
        "                model: %4\n"
        "                Row {\n"
        "                    spacing: 6\n"
        "                    Rectangle {\n"
        "                        width: 14\n"
        "                        height: 14\n"
        "                        color: %5[index % %5.length]\n"
        "                        radius: 2\n"
        "                    }\n"
        "                    Text {\n"
        "                        text: modelData\n"
        "                        color: \"#ccc\"\n"
        "                        font.pixelSize: 14\n"
        "                    }\n"
        "                }\n"
        "            }\n"
        "        }\n"
        "    }\n"
        "}\n"
    ).arg(preferredHeight).arg(title).arg(valuesStr).arg(labelsStr).arg(colorsStr);
}

// Остальные диаграммы (ChartBar, ChartBarCompare, ChartLine) 
// остаются без изменений из предыдущей версии
QString generateBarChart(const QJsonObject& spec)
{
    QString title = spec["title"].toString("Диаграмма");
    int preferredHeight = spec["height"].toInt(350);
    if (preferredHeight <= 0) preferredHeight = 350;

    QJsonArray values;
    QStringList labels;
    parseChartData(spec, values, labels);

    QStringList colors = {
        "#ff6b6b", "#feca57", "#48dbfb", "#ff9ff3", "#54a0ff",
        "#1dd1a1", "#f368e0", "#00d2d3", "#ff9f43", "#a29bfe"
    };

    QString labelsStr = "[";
    QString valuesStr = "[";
    QString colorsStr = "[";
    for (int i = 0; i < values.size(); ++i) {
        if (i > 0) {
            labelsStr += ", ";
            valuesStr += ", ";
            colorsStr += ", ";
        }
        double val = values[i].toDouble();
        labelsStr += "\"" + labels[i] + "\"";
        valuesStr += QString::number(val);
        colorsStr += "\"" + colors[i % colors.size()] + "\"";
    }
    labelsStr += "]";
    valuesStr += "]";
    colorsStr += "]";

    return QString(
        "Rectangle {\n"
        "    color: \"transparent\"\n"
        "    Layout.fillWidth: true\n"
        "    Layout.preferredHeight: %1\n"
        "    ColumnLayout {\n"
        "        anchors.fill: parent\n"
        "        Text {\n"
        "            text: \"%2\"\n"
        "            color: \"#aaa\"\n"
        "            font.pixelSize: 14\n"
        "            font.bold: true\n"
        "            Layout.alignment: Qt.AlignHCenter\n"
        "            Layout.preferredHeight: implicitHeight\n"
        "        }\n"
        "        Rectangle {\n"
        "            Layout.fillWidth: true\n"
        "            Layout.fillHeight: true\n"
        "            color: \"transparent\"\n"
        "            border.color: \"#444\"\n"
        "            border.width: 1\n"
        "            radius: 4\n"
        "            Canvas {\n"
        "                id: barCanvas\n"
        "                anchors.fill: parent\n"
        "                anchors.margins: 10\n"
        "                property var data: %4\n"
        "                property var labels: %3\n"
        "                property var colors: %5\n"
        "                property int hoveredIndex: -1\n"
        "                \n"
        "                onPaint: {\n"
        "                    if (!data || data.length === 0) return;\n"
        "                    var ctx = getContext(\"2d\");\n"
        "                    var maxVal = Math.max.apply(null, data);\n"
        "                    if (maxVal === 0) maxVal = 1;\n"
        "                    var barWidth = width / data.length * 0.7;\n"
        "                    var gap = (width / data.length - barWidth) / 2;\n"
        "                    for (var i = 0; i < data.length; i++) {\n"
        "                        var isHovered = (i === hoveredIndex);\n"
        "                        var h = (data[i] / maxVal) * (height - 20);\n"
        "                        var x = i * (barWidth + gap * 2) + gap;\n"
        "                        var y = height - h - 10;\n"
        "                        var barW = isHovered ? barWidth * 1.15 : barWidth;\n"
        "                        var offsetX = isHovered ? (barWidth * 1.15 - barWidth) / 2 : 0;\n"
        "                        ctx.fillStyle = colors[i % colors.length];\n"
        "                        ctx.fillRect(x - offsetX, y, barW, h);\n"
        "                        ctx.strokeStyle = \"#333\";\n"
        "                        ctx.lineWidth = isHovered ? 3 : 1;\n"
        "                        ctx.strokeRect(x - offsetX, y, barW, h);\n"
        "                    }\n"
        "                }\n"
        "                \n"
        "                Component.onCompleted: {\n"
        "                    requestPaint()\n"
        "                }\n"
        "                onDataChanged: requestPaint()\n"
        "                onWidthChanged: requestPaint()\n"
        "                onHeightChanged: requestPaint()\n"
        "            }\n"
        "        }\n"
        "        Flow {\n"
        "            Layout.fillWidth: true\n"
        "            Layout.preferredHeight: implicitHeight\n"
        "            spacing: 20\n"
        "            padding: 8\n"
        "            Repeater {\n"
        "                model: %3\n"
        "                Row {\n"
        "                    spacing: 6\n"
        "                    Rectangle {\n"
        "                        width: 14\n"
        "                        height: 14\n"
        "                        color: %5[index % %5.length]\n"
        "                        radius: 2\n"
        "                    }\n"
        "                    Text {\n"
        "                        text: modelData\n"
        "                        color: \"#ccc\"\n"
        "                        font.pixelSize: 14\n"
        "                    }\n"
        "                }\n"
        "            }\n"
        "        }\n"
        "    }\n"
        "}\n"
    ).arg(preferredHeight).arg(title).arg(labelsStr).arg(valuesStr).arg(colorsStr);
}

QString generateBarCompareChart(const QJsonObject& spec)
{
    QString title = spec["title"].toString("Сравнение");
    int preferredHeight = spec["height"].toInt(350);
    if (preferredHeight <= 0) preferredHeight = 350;

    QJsonArray values1;
    QStringList labels1;
    parseChartData(spec, values1, labels1);

    QJsonArray values2;
    if (spec.contains("data2") && spec["data2"].isObject()) {
        QJsonObject data2Obj = spec["data2"].toObject();
        if (data2Obj.contains("Marked_values") && data2Obj["Marked_values"].isArray()) {
            QJsonArray markedValues = data2Obj["Marked_values"].toArray();
            for (const QJsonValue& item : markedValues) {
                if (item.isObject()) {
                    QJsonObject obj = item.toObject();
                    if (obj.contains("value")) {
                        values2.append(obj["value"].toDouble());
                    }
                }
            }
        }
    }
    if (values2.isEmpty()) {
        values2 = {40, 70, 50, 80, 55};
    }

    QString valuesStr1 = "[";
    QString valuesStr2 = "[";
    for (int i = 0; i < values1.size(); ++i) {
        if (i > 0) valuesStr1 += ", ";
        valuesStr1 += QString::number(values1[i].toDouble());
    }
    for (int i = 0; i < values2.size(); ++i) {
        if (i > 0) valuesStr2 += ", ";
        valuesStr2 += QString::number(values2[i].toDouble());
    }
    valuesStr1 += "]";
    valuesStr2 += "]";

    return QString(
        "Rectangle {\n"
        "    color: \"transparent\"\n"
        "    Layout.fillWidth: true\n"
        "    Layout.preferredHeight: %1\n"
        "    ColumnLayout {\n"
        "        anchors.fill: parent\n"
        "        Text {\n"
        "            text: \"%2\"\n"
        "            color: \"#aaa\"\n"
        "            font.pixelSize: 14\n"
        "            font.bold: true\n"
        "            Layout.alignment: Qt.AlignHCenter\n"
        "            Layout.preferredHeight: implicitHeight\n"
        "        }\n"
        "        Rectangle {\n"
        "            Layout.fillWidth: true\n"
        "            Layout.fillHeight: true\n"
        "            color: \"transparent\"\n"
        "            border.color: \"#444\"\n"
        "            border.width: 1\n"
        "            radius: 4\n"
        "            Canvas {\n"
        "                id: compareCanvas\n"
        "                anchors.fill: parent\n"
        "                anchors.margins: 10\n"
        "                property var data1: %3\n"
        "                property var data2: %4\n"
        "                \n"
        "                onPaint: {\n"
        "                    if (!data1 || !data2 || data1.length === 0) return;\n"
        "                    var ctx = getContext(\"2d\");\n"
        "                    var maxVal = Math.max.apply(null, data1.concat(data2));\n"
        "                    if (maxVal === 0) maxVal = 1;\n"
        "                    var groupWidth = width / data1.length * 0.7;\n"
        "                    var barWidth = groupWidth / 2 - 2;\n"
        "                    var gap = (width / data1.length - groupWidth) / 2;\n"
        "                    var colors1 = [\"#ff6b6b\", \"#feca57\", \"#48dbfb\", \"#ff9ff3\", \"#54a0ff\"];\n"
        "                    var colors2 = [\"#ff9f43\", \"#ff6b6b\", \"#1dd1a1\", \"#f368e0\", \"#00d2d3\"];\n"
        "                    for (var i = 0; i < data1.length; i++) {\n"
        "                        var h1 = (data1[i] / maxVal) * (height - 20);\n"
        "                        var h2 = (data2[i] / maxVal) * (height - 20);\n"
        "                        var x = i * (groupWidth + gap * 2) + gap;\n"
        "                        var y1 = height - h1 - 10;\n"
        "                        var y2 = height - h2 - 10;\n"
        "                        ctx.fillStyle = colors1[i % colors1.length];\n"
        "                        ctx.fillRect(x, y1, barWidth, h1);\n"
        "                        ctx.strokeStyle = \"#333\";\n"
        "                        ctx.lineWidth = 1;\n"
        "                        ctx.strokeRect(x, y1, barWidth, h1);\n"
        "                        ctx.fillStyle = colors2[i % colors2.length];\n"
        "                        ctx.fillRect(x + barWidth + 2, y2, barWidth, h2);\n"
        "                        ctx.strokeStyle = \"#333\";\n"
        "                        ctx.lineWidth = 1;\n"
        "                        ctx.strokeRect(x + barWidth + 2, y2, barWidth, h2);\n"
        "                    }\n"
        "                }\n"
        "                \n"
        "                Component.onCompleted: requestPaint()\n"
        "                onData1Changed: requestPaint()\n"
        "                onData2Changed: requestPaint()\n"
        "                onWidthChanged: requestPaint()\n"
        "                onHeightChanged: requestPaint()\n"
        "            }\n"
        "        }\n"
        "        Flow {\n"
        "            Layout.fillWidth: true\n"
        "            Layout.preferredHeight: implicitHeight\n"
        "            spacing: 20\n"
        "            padding: 8\n"
        "            Row {\n"
        "                spacing: 6\n"
        "                Rectangle { width: 14; height: 14; color: \"#ff6b6b\"; radius: 2 }\n"
        "                Text { text: \"Ряд 1 (план)\"; color: \"#ccc\"; font.pixelSize: 14 }\n"
        "            }\n"
        "            Row {\n"
        "                spacing: 6\n"
        "                Rectangle { width: 14; height: 14; color: \"#ff9f43\"; radius: 2 }\n"
        "                Text { text: \"Ряд 2 (факт)\"; color: \"#ccc\"; font.pixelSize: 14 }\n"
        "            }\n"
        "        }\n"
        "    }\n"
        "}\n"
    ).arg(preferredHeight).arg(title).arg(valuesStr1).arg(valuesStr2);
}

QString generateLineChart(const QJsonObject& spec)
{
    QString title = spec["title"].toString("График");
    int preferredHeight = spec["height"].toInt(350);
    if (preferredHeight <= 0) preferredHeight = 350;

    QJsonArray values;
    QStringList labels;
    parseChartData(spec, values, labels);

    QString valuesStr = "[";
    QString labelsStr = "[";
    for (int i = 0; i < values.size(); ++i) {
        if (i > 0) {
            valuesStr += ", ";
            labelsStr += ", ";
        }
        double val = values[i].toDouble();
        valuesStr += QString::number(val);
        labelsStr += "\"" + labels[i] + "\"";
    }
    valuesStr += "]";
    labelsStr += "]";

    return QString(
        "Rectangle {\n"
        "    color: \"transparent\"\n"
        "    Layout.fillWidth: true\n"
        "    Layout.preferredHeight: %1\n"
        "    ColumnLayout {\n"
        "        anchors.fill: parent\n"
        "        Text {\n"
        "            text: \"%2\"\n"
        "            color: \"#aaa\"\n"
        "            font.pixelSize: 14\n"
        "            font.bold: true\n"
        "            Layout.alignment: Qt.AlignHCenter\n"
        "            Layout.preferredHeight: implicitHeight\n"
        "        }\n"
        "        Rectangle {\n"
        "            Layout.fillWidth: true\n"
        "            Layout.fillHeight: true\n"
        "            color: \"transparent\"\n"
        "            border.color: \"#444\"\n"
        "            border.width: 1\n"
        "            radius: 4\n"
        "            Canvas {\n"
        "                id: lineCanvas\n"
        "                anchors.fill: parent\n"
        "                anchors.margins: 10\n"
        "                property var data: %3\n"
        "                property var labels: %4\n"
        "                \n"
        "                onPaint: {\n"
        "                    if (!data || data.length === 0) return;\n"
        "                    var ctx = getContext(\"2d\");\n"
        "                    var maxVal = Math.max.apply(null, data);\n"
        "                    if (maxVal === 0) maxVal = 1;\n"
        "                    var stepX = width / (data.length - 1);\n"
        "                    ctx.beginPath();\n"
        "                    ctx.strokeStyle = \"#48dbfb\";\n"
        "                    ctx.lineWidth = 3;\n"
        "                    for (var i = 0; i < data.length; i++) {\n"
        "                        var x = i * stepX;\n"
        "                        var y = height - (data[i] / maxVal) * (height - 20) - 10;\n"
        "                        if (i === 0) ctx.moveTo(x, y);\n"
        "                        else ctx.lineTo(x, y);\n"
        "                    }\n"
        "                    ctx.stroke();\n"
        "                    ctx.fillStyle = \"#48dbfb\";\n"
        "                    for (var i = 0; i < data.length; i++) {\n"
        "                        var x = i * stepX;\n"
        "                        var y = height - (data[i] / maxVal) * (height - 20) - 10;\n"
        "                        ctx.beginPath();\n"
        "                        ctx.arc(x, y, 5, 0, 2 * Math.PI);\n"
        "                        ctx.fill();\n"
        "                    }\n"
        "                }\n"
        "                \n"
        "                Component.onCompleted: requestPaint()\n"
        "                onDataChanged: requestPaint()\n"
        "                onWidthChanged: requestPaint()\n"
        "                onHeightChanged: requestPaint()\n"
        "            }\n"
        "        }\n"
        "        Flow {\n"
        "            Layout.fillWidth: true\n"
        "            Layout.preferredHeight: implicitHeight\n"
        "            spacing: 20\n"
        "            padding: 8\n"
        "            Row {\n"
        "                spacing: 6\n"
        "                Rectangle { width: 14; height: 14; color: \"#48dbfb\"; radius: 2 }\n"
        "                Text { text: \"Значения\"; color: \"#ccc\"; font.pixelSize: 14 }\n"
        "            }\n"
        "        }\n"
        "    }\n"
        "}\n"
    ).arg(preferredHeight).arg(title).arg(valuesStr).arg(labelsStr);
}
