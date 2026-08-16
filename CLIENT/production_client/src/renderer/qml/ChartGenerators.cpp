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

    QStringList legendItems;
    for (int i = 0; i < values.size(); ++i) {
        double val = values[i].toDouble();
        legendItems << QString("%1 (%2)").arg(labels[i]).arg(val);
    }

    QString valuesStr = "[";
    QString labelsStr = "[";
    QString colorsStr = "[";
    QString legendStr = "[";
    for (int i = 0; i < values.size(); ++i) {
        if (i > 0) {
            valuesStr += ", ";
            labelsStr += ", ";
            colorsStr += ", ";
            legendStr += ", ";
        }
        double val = values[i].toDouble();
        valuesStr += QString::number(val);
        labelsStr += "\"" + labels[i] + "\"";
        colorsStr += "\"" + colors[i % colors.size()] + "\"";
        legendStr += "\"" + legendItems[i] + "\"";
    }
    valuesStr += "]";
    labelsStr += "]";
    colorsStr += "]";
    legendStr += "]";

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
        "                objectName: \"pieCanvas\"\n"
        "                anchors.fill: parent\n"
        "                anchors.margins: 10\n"
        "                property var chartData: %3\n"
        "                property var chartLabels: %4\n"
        "                property var chartColors: %5\n"
        "                property int hoveredIndex: -1\n"
        "                \n"
        "                onPaint: {\n"
        "                    if (!chartData || chartData.length === 0) {\n"
        "                        var ctx = getContext(\"2d\");\n"
        "                        ctx.fillStyle = \"#2a2a2a\";\n"
        "                        ctx.fillRect(0, 0, width, height);\n"
        "                        ctx.fillStyle = \"#666\";\n"
        "                        ctx.font = \"16px sans-serif\";\n"
        "                        ctx.textAlign = \"center\";\n"
        "                        ctx.fillText(\"Нет данных\", width/2, height/2);\n"
        "                        return;\n"
        "                    }\n"
        "                    if (width <= 10 || height <= 10) return;\n"
        "                    \n"
        "                    var ctx = getContext(\"2d\");\n"
        "                    ctx.clearRect(0, 0, width, height);\n"
        "                    \n"
        "                    var cx = width / 2;\n"
        "                    var cy = height / 2;\n"
        "                    var r = Math.min(width, height) / 2 - 20;\n"
        "                    if (r < 10) r = 10;\n"
        "                    var total = 0;\n"
        "                    for (var i = 0; i < chartData.length; i++) total += chartData[i];\n"
        "                    if (total === 0) total = 1;\n"
        "                    var startAngle = 0;\n"
        "                    for (var i = 0; i < chartData.length; i++) {\n"
        "                        var angle = (chartData[i] / total) * 2 * Math.PI;\n"
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
        "                        ctx.fillStyle = chartColors[i % chartColors.length];\n"
        "                        ctx.fill();\n"
        "                        ctx.strokeStyle = \"#333\";\n"
        "                        ctx.lineWidth = isHovered ? 3 : 2;\n"
        "                        ctx.stroke();\n"
        "                        startAngle += angle;\n"
        "                    }\n"
        "                }\n"
        "                \n"
        "                function getSectorAt(mx, my) {\n"
        "                    if (!chartData || chartData.length === 0) return -1;\n"
        "                    var cx = width / 2;\n"
        "                    var cy = height / 2;\n"
        "                    var r = Math.min(width, height) / 2 - 20;\n"
        "                    if (r < 10) r = 10;\n"
        "                    var dx = mx - cx;\n"
        "                    var dy = my - cy;\n"
        "                    var dist = Math.sqrt(dx*dx + dy*dy);\n"
        "                    if (dist > r * 1.08) return -1;\n"
        "                    var angle = Math.atan2(dy, dx);\n"
        "                    if (angle < 0) angle += 2 * Math.PI;\n"
        "                    var total = 0;\n"
        "                    for (var i = 0; i < chartData.length; i++) total += chartData[i];\n"
        "                    if (total === 0) total = 1;\n"
        "                    var startAngle = 0;\n"
        "                    for (var i = 0; i < chartData.length; i++) {\n"
        "                        var a = (chartData[i] / total) * 2 * Math.PI;\n"
        "                        if (angle >= startAngle && angle < startAngle + a) return i;\n"
        "                        startAngle += a;\n"
        "                    }\n"
        "                    return -1;\n"
        "                }\n"
        "                \n"
        "                MouseArea {\n"
        "                    anchors.fill: parent\n"
        "                    hoverEnabled: true\n"
        "                    onPositionChanged: {\n"
        "                        var idx = parent.getSectorAt(mouseX, mouseY);\n"
        "                        if (idx !== parent.hoveredIndex) {\n"
        "                            parent.hoveredIndex = idx;\n"
        "                            parent.requestPaint();\n"
        "                            if (idx >= 0) {\n"
        "                                tooltipText.text = parent.chartData[idx] + \" - \" + parent.chartLabels[idx];\n"
        "                                tooltip.x = mouseX + 15;\n"
        "                                tooltip.y = mouseY - 10;\n"
        "                                tooltip.visible = true;\n"
        "                            } else {\n"
        "                                tooltip.visible = false;\n"
        "                            }\n"
        "                        }\n"
        "                    }\n"
        "                    onExited: {\n"
        "                        parent.hoveredIndex = -1;\n"
        "                        parent.requestPaint();\n"
        "                        tooltip.visible = false;\n"
        "                    }\n"
        "                }\n"
        "                \n"
        "                Rectangle {\n"
        "                    id: tooltip\n"
        "                    visible: false\n"
        "                    color: \"#2d2d2d\"\n"
        "                    border.color: \"#555\"\n"
        "                    border.width: 1\n"
        "                    radius: 4\n"
        "                    width: tooltipText.width + 16;\n"
        "                    height: tooltipText.height + 16;\n"
        "                    x: 0;\n"
        "                    y: 0;\n"
        "                    z: 10;\n"
        "                    Text {\n"
        "                        id: tooltipText\n"
        "                        color: \"#ffffff\"\n"
        "                        font.pixelSize: 12;\n"
        "                        anchors.centerIn: parent;\n"
        "                    }\n"
        "                }\n"
        "                \n"
        "                Component.onCompleted: requestPaint();\n"
        "                onChartDataChanged: requestPaint();\n"
        "                onWidthChanged: { if (width > 10) requestPaint(); }\n"
        "                onHeightChanged: { if (height > 10) requestPaint(); }\n"
        "            }\n"
        "        }\n"
        "        Flow {\n"
        "            Layout.fillWidth: true\n"
        "            Layout.preferredHeight: implicitHeight\n"
        "            spacing: 20\n"
        "            padding: 8\n"
        "            Repeater {\n"
        "                model: %6\n"
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
    ).arg(preferredHeight).arg(title).arg(valuesStr).arg(labelsStr).arg(colorsStr).arg(legendStr);
}

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

    QStringList legendItems;
    for (int i = 0; i < values.size(); ++i) {
        double val = values[i].toDouble();
        legendItems << QString("%1 (%2)").arg(labels[i]).arg(val);
    }

    QString labelsStr = "[";
    QString valuesStr = "[";
    QString colorsStr = "[";
    QString legendStr = "[";
    for (int i = 0; i < values.size(); ++i) {
        if (i > 0) {
            labelsStr += ", ";
            valuesStr += ", ";
            colorsStr += ", ";
            legendStr += ", ";
        }
        double val = values[i].toDouble();
        labelsStr += "\"" + labels[i] + "\"";
        valuesStr += QString::number(val);
        colorsStr += "\"" + colors[i % colors.size()] + "\"";
        legendStr += "\"" + legendItems[i] + "\"";
    }
    labelsStr += "]";
    valuesStr += "]";
    colorsStr += "]";
    legendStr += "]";

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
        "                objectName: \"barCanvas\"\n"
        "                anchors.fill: parent\n"
        "                anchors.margins: 10\n"
        "                property var chartData: %4\n"
        "                property var chartLabels: %3\n"
        "                property var chartColors: %5\n"
        "                property int hoveredIndex: -1\n"
        "                \n"
        "                onPaint: {\n"
        "                    if (!chartData || chartData.length === 0) return;\n"
        "                    var ctx = getContext(\"2d\");\n"
        "                    ctx.clearRect(0, 0, width, height);\n"
        "                    var maxVal = Math.max.apply(null, chartData);\n"
        "                    if (maxVal === 0) maxVal = 1;\n"
        "                    var barWidth = width / chartData.length * 0.7;\n"
        "                    var gap = (width / chartData.length - barWidth) / 2;\n"
        "                    for (var i = 0; i < chartData.length; i++) {\n"
        "                        var isHovered = (i === hoveredIndex);\n"
        "                        var h = (chartData[i] / maxVal) * (height - 20);\n"
        "                        var x = i * (barWidth + gap * 2) + gap;\n"
        "                        var y = height - h - 10;\n"
        "                        var barW = isHovered ? barWidth * 1.15 : barWidth;\n"
        "                        var offsetX = isHovered ? (barW - barWidth) / 2 : 0;\n"
        "                        ctx.fillStyle = chartColors[i % chartColors.length];\n"
        "                        ctx.fillRect(x - offsetX, y, barW, h);\n"
        "                        ctx.strokeStyle = \"#333\";\n"
        "                        ctx.lineWidth = isHovered ? 3 : 1;\n"
        "                        ctx.strokeRect(x - offsetX, y, barW, h);\n"
        "                    }\n"
        "                }\n"
        "                \n"
        "                function getBarAt(mx, my) {\n"
        "                    if (!chartData || chartData.length === 0) return -1;\n"
        "                    var maxVal = Math.max.apply(null, chartData);\n"
        "                    if (maxVal === 0) maxVal = 1;\n"
        "                    var barWidth = width / chartData.length * 0.7;\n"
        "                    var gap = (width / chartData.length - barWidth) / 2;\n"
        "                    for (var i = 0; i < chartData.length; i++) {\n"
        "                        var h = (chartData[i] / maxVal) * (height - 20);\n"
        "                        var x = i * (barWidth + gap * 2) + gap;\n"
        "                        var y = height - h - 10;\n"
        "                        if (mx >= x && mx <= x + barWidth && my >= y && my <= y + h) return i;\n"
        "                    }\n"
        "                    return -1;\n"
        "                }\n"
        "                \n"
        "                MouseArea {\n"
        "                    anchors.fill: parent\n"
        "                    hoverEnabled: true\n"
        "                    onPositionChanged: {\n"
        "                        var idx = parent.getBarAt(mouseX, mouseY);\n"
        "                        if (idx !== parent.hoveredIndex) {\n"
        "                            parent.hoveredIndex = idx;\n"
        "                            parent.requestPaint();\n"
        "                            if (idx >= 0) {\n"
        "                                tooltipText.text = parent.chartData[idx] + \" - \" + parent.chartLabels[idx];\n"
        "                                tooltip.x = mouseX + 15;\n"
        "                                tooltip.y = mouseY - 10;\n"
        "                                tooltip.visible = true;\n"
        "                            } else {\n"
        "                                tooltip.visible = false;\n"
        "                            }\n"
        "                        }\n"
        "                    }\n"
        "                    onExited: {\n"
        "                        parent.hoveredIndex = -1;\n"
        "                        parent.requestPaint();\n"
        "                        tooltip.visible = false;\n"
        "                    }\n"
        "                }\n"
        "                \n"
        "                Rectangle {\n"
        "                    id: tooltip\n"
        "                    visible: false\n"
        "                    color: \"#2d2d2d\"\n"
        "                    border.color: \"#555\"\n"
        "                    border.width: 1\n"
        "                    radius: 4\n"
        "                    width: tooltipText.width + 16;\n"
        "                    height: tooltipText.height + 16;\n"
        "                    x: 0;\n"
        "                    y: 0;\n"
        "                    z: 10;\n"
        "                    Text {\n"
        "                        id: tooltipText\n"
        "                        color: \"#ffffff\"\n"
        "                        font.pixelSize: 12;\n"
        "                        anchors.centerIn: parent;\n"
        "                    }\n"
        "                }\n"
        "                \n"
        "                Component.onCompleted: requestPaint();\n"
        "                onChartDataChanged: requestPaint();\n"
        "                onWidthChanged: requestPaint();\n"
        "                onHeightChanged: requestPaint();\n"
        "            }\n"
        "        }\n"
        "        Flow {\n"
        "            Layout.fillWidth: true\n"
        "            Layout.preferredHeight: implicitHeight\n"
        "            spacing: 20\n"
        "            padding: 8\n"
        "            Repeater {\n"
        "                model: %6\n"
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
    ).arg(preferredHeight).arg(title).arg(labelsStr).arg(valuesStr).arg(colorsStr).arg(legendStr);
}

QString generateBarCompareChart(const QJsonObject& spec)
{
    QString title = spec["title"].toString("Сравнение");
    int preferredHeight = spec["height"].toInt(350);
    if (preferredHeight <= 0) preferredHeight = 350;

    QJsonArray values1;
    QStringList labels;
    parseChartData(spec, values1, labels);

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

    // Формируем строки для передачи в QML
    QString valuesStr1 = "[";
    QString valuesStr2 = "[";
    QString labelsStr = "[";
    for (int i = 0; i < values1.size(); ++i) {
        if (i > 0) {
            valuesStr1 += ", ";
            valuesStr2 += ", ";
            labelsStr += ", ";
        }
        valuesStr1 += QString::number(values1[i].toDouble());
        valuesStr2 += QString::number(values2[i].toDouble());
        labelsStr += "\"" + (i < labels.size() ? labels[i] : "Категория " + QString::number(i+1)) + "\"";
    }
    valuesStr1 += "]";
    valuesStr2 += "]";
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
        "                id: compareCanvas\n"
        "                objectName: \"compareCanvas\"\n"
        "                anchors.fill: parent\n"
        "                anchors.margins: 10\n"
        "                property var chartData1: %3\n"
        "                property var chartData2: %4\n"
        "                property var chartLabels: %5\n"
        "                property int hoveredIndex: -1\n"
        "                \n"
        "                onPaint: {\n"
        "                    if (!chartData1 || !chartData2 || chartData1.length === 0) return;\n"
        "                    var ctx = getContext(\"2d\");\n"
        "                    ctx.clearRect(0, 0, width, height);\n"
        "                    var maxVal = Math.max.apply(null, chartData1.concat(chartData2));\n"
        "                    if (maxVal === 0) maxVal = 1;\n"
        "                    var groupWidth = width / chartData1.length * 0.7;\n"
        "                    var barWidth = groupWidth / 2 - 2;\n"
        "                    var gap = (width / chartData1.length - groupWidth) / 2;\n"
        "                    var colors1 = [\"#ff6b6b\", \"#feca57\", \"#48dbfb\", \"#ff9ff3\", \"#54a0ff\"];\n"
        "                    var colors2 = [\"#ff9f43\", \"#ff6b6b\", \"#1dd1a1\", \"#f368e0\", \"#00d2d3\"];\n"
        "                    for (var i = 0; i < chartData1.length; i++) {\n"
        "                        var isHovered = (i === hoveredIndex);\n"
        "                        var h1 = (chartData1[i] / maxVal) * (height - 20);\n"
        "                        var h2 = (chartData2[i] / maxVal) * (height - 20);\n"
        "                        var x = i * (groupWidth + gap * 2) + gap;\n"
        "                        var y1 = height - h1 - 10;\n"
        "                        var y2 = height - h2 - 10;\n"
        "                        \n"
        "                        var barW1 = isHovered ? barWidth * 1.15 : barWidth;\n"
        "                        var offsetX1 = isHovered ? (barW1 - barWidth) / 2 : 0;\n"
        "                        ctx.fillStyle = colors1[i % colors1.length];\n"
        "                        ctx.fillRect(x - offsetX1, y1, barW1, h1);\n"
        "                        ctx.strokeStyle = \"#333\";\n"
        "                        ctx.lineWidth = isHovered ? 3 : 1;\n"
        "                        ctx.strokeRect(x - offsetX1, y1, barW1, h1);\n"
        "                        \n"
        "                        var barW2 = isHovered ? barWidth * 1.15 : barWidth;\n"
        "                        var offsetX2 = isHovered ? (barW2 - barWidth) / 2 : 0;\n"
        "                        ctx.fillStyle = colors2[i % colors2.length];\n"
        "                        ctx.fillRect(x + barWidth + 2 - offsetX2, y2, barW2, h2);\n"
        "                        ctx.strokeStyle = \"#333\";\n"
        "                        ctx.lineWidth = isHovered ? 3 : 1;\n"
        "                        ctx.strokeRect(x + barWidth + 2 - offsetX2, y2, barW2, h2);\n"
        "                    }\n"
        "                }\n"
        "                \n"
        "                function getBarAt(mx, my) {\n"
        "                    if (!chartData1 || !chartData2 || chartData1.length === 0) return -1;\n"
        "                    var maxVal = Math.max.apply(null, chartData1.concat(chartData2));\n"
        "                    if (maxVal === 0) maxVal = 1;\n"
        "                    var groupWidth = width / chartData1.length * 0.7;\n"
        "                    var barWidth = groupWidth / 2 - 2;\n"
        "                    var gap = (width / chartData1.length - groupWidth) / 2;\n"
        "                    for (var i = 0; i < chartData1.length; i++) {\n"
        "                        var h1 = (chartData1[i] / maxVal) * (height - 20);\n"
        "                        var h2 = (chartData2[i] / maxVal) * (height - 20);\n"
        "                        var x = i * (groupWidth + gap * 2) + gap;\n"
        "                        var y1 = height - h1 - 10;\n"
        "                        var y2 = height - h2 - 10;\n"
        "                        if (mx >= x && mx <= x + barWidth && my >= y1 && my <= y1 + h1) return i;\n"
        "                        if (mx >= x + barWidth + 2 && mx <= x + barWidth + 2 + barWidth && my >= y2 && my <= y2 + h2) return i;\n"
        "                    }\n"
        "                    return -1;\n"
        "                }\n"
        "                \n"
        "                MouseArea {\n"
        "                    anchors.fill: parent\n"
        "                    hoverEnabled: true\n"
        "                    onPositionChanged: {\n"
        "                        var idx = parent.getBarAt(mouseX, mouseY);\n"
        "                        if (idx !== parent.hoveredIndex) {\n"
        "                            parent.hoveredIndex = idx;\n"
        "                            parent.requestPaint();\n"
        "                            if (idx >= 0) {\n"
        "                                var label = parent.chartLabels && parent.chartLabels.length > idx ? parent.chartLabels[idx] : \"\";\n"
        "                                tooltipText.text = \"Ряд 1: \" + parent.chartData1[idx] + \"  Ряд 2: \" + parent.chartData2[idx] + (label ? \"  (\" + label + \")\" : \"\");\n"
        "                                tooltip.x = mouseX + 15;\n"
        "                                tooltip.y = mouseY - 10;\n"
        "                                tooltip.visible = true;\n"
        "                            } else {\n"
        "                                tooltip.visible = false;\n"
        "                            }\n"
        "                        }\n"
        "                    }\n"
        "                    onExited: {\n"
        "                        parent.hoveredIndex = -1;\n"
        "                        parent.requestPaint();\n"
        "                        tooltip.visible = false;\n"
        "                    }\n"
        "                }\n"
        "                \n"
        "                Rectangle {\n"
        "                    id: tooltip\n"
        "                    visible: false\n"
        "                    color: \"#2d2d2d\"\n"
        "                    border.color: \"#555\"\n"
        "                    border.width: 1\n"
        "                    radius: 4\n"
        "                    width: tooltipText.width + 16;\n"
        "                    height: tooltipText.height + 16;\n"
        "                    x: 0;\n"
        "                    y: 0;\n"
        "                    z: 10;\n"
        "                    Text {\n"
        "                        id: tooltipText\n"
        "                        color: \"#ffffff\"\n"
        "                        font.pixelSize: 12;\n"
        "                        anchors.centerIn: parent;\n"
        "                    }\n"
        "                }\n"
        "                \n"
        "                Component.onCompleted: requestPaint();\n"
        "                onChartData1Changed: requestPaint();\n"
        "                onChartData2Changed: requestPaint();\n"
        "                onWidthChanged: requestPaint();\n"
        "                onHeightChanged: requestPaint();\n"
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
    ).arg(preferredHeight).arg(title).arg(valuesStr1).arg(valuesStr2).arg(labelsStr);
}

QString generateLineChart(const QJsonObject& spec)
{
    QString title = spec["title"].toString("График");
    int preferredHeight = spec["height"].toInt(350);
    if (preferredHeight <= 0) preferredHeight = 350;

    QJsonArray values;
    QStringList labels;
    parseChartData(spec, values, labels);

    QStringList legendItems;
    for (int i = 0; i < values.size(); ++i) {
        double val = values[i].toDouble();
        legendItems << QString("%1 (%2)").arg(labels[i]).arg(val);
    }

    QString valuesStr = "[";
    QString labelsStr = "[";
    QString legendStr = "[";
    for (int i = 0; i < values.size(); ++i) {
        if (i > 0) {
            valuesStr += ", ";
            labelsStr += ", ";
            legendStr += ", ";
        }
        double val = values[i].toDouble();
        valuesStr += QString::number(val);
        labelsStr += "\"" + labels[i] + "\"";
        legendStr += "\"" + legendItems[i] + "\"";
    }
    valuesStr += "]";
    labelsStr += "]";
    legendStr += "]";

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
        "                objectName: \"lineCanvas\"\n"
        "                anchors.fill: parent\n"
        "                anchors.margins: 10\n"
        "                property var chartData: %3\n"
        "                property var chartLabels: %4\n"
        "                \n"
        "                onPaint: {\n"
        "                    if (!chartData || chartData.length === 0) return;\n"
        "                    var ctx = getContext(\"2d\");\n"
        "                    ctx.clearRect(0, 0, width, height);\n"
        "                    var maxVal = Math.max.apply(null, chartData);\n"
        "                    if (maxVal === 0) maxVal = 1;\n"
        "                    var stepX = width / (chartData.length - 1);\n"
        "                    ctx.beginPath();\n"
        "                    ctx.strokeStyle = \"#48dbfb\";\n"
        "                    ctx.lineWidth = 3;\n"
        "                    for (var i = 0; i < chartData.length; i++) {\n"
        "                        var x = i * stepX;\n"
        "                        var y = height - (chartData[i] / maxVal) * (height - 20) - 10;\n"
        "                        if (i === 0) ctx.moveTo(x, y);\n"
        "                        else ctx.lineTo(x, y);\n"
        "                    }\n"
        "                    ctx.stroke();\n"
        "                    ctx.fillStyle = \"#48dbfb\";\n"
        "                    for (var i = 0; i < chartData.length; i++) {\n"
        "                        var x = i * stepX;\n"
        "                        var y = height - (chartData[i] / maxVal) * (height - 20) - 10;\n"
        "                        ctx.beginPath();\n"
        "                        ctx.arc(x, y, 5, 0, 2 * Math.PI);\n"
        "                        ctx.fill();\n"
        "                    }\n"
        "                }\n"
        "                Component.onCompleted: requestPaint();\n"
        "                onChartDataChanged: requestPaint();\n"
        "                onWidthChanged: requestPaint();\n"
        "                onHeightChanged: requestPaint();\n"
        "            }\n"
        "        }\n"
        "        Flow {\n"
        "            Layout.fillWidth: true\n"
        "            Layout.preferredHeight: implicitHeight\n"
        "            spacing: 20\n"
        "            padding: 8\n"
        "            Repeater {\n"
        "                model: %5\n"
        "                Row {\n"
        "                    spacing: 6\n"
        "                    Rectangle {\n"
        "                        width: 14\n"
        "                        height: 14\n"
        "                        color: \"#48dbfb\"\n"
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
    ).arg(preferredHeight).arg(title).arg(valuesStr).arg(labelsStr).arg(legendStr);
}
