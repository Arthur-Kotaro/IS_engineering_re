#include "QmlObjectFactory.h"
#include <QQmlEngine>
#include <QQmlComponent>
#include <QQuickItem>
#include <QDebug>
#include <QJsonArray>

QmlObjectFactory::QmlObjectFactory(QQmlEngine* engine, QObject* parent)
    : QObject(parent)
    , m_engine(engine)
{
    qDebug() << "QmlObjectFactory::QmlObjectFactory()";
    registerBuiltInTypes();
    qDebug() << "  Registered" << m_qmlGenerators.size() << "widget types";
}

// ============================================================
// ВСПОМОГАТЕЛЬНАЯ ФУНКЦИЯ ДЛЯ ПАРСИНГА ДАННЫХ
// ============================================================
static void parseChartData(const QJsonObject& spec, QJsonArray& values, QStringList& labels)
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

void QmlObjectFactory::registerBuiltInTypes()
{
    qDebug() << "  Registering built-in widget types...";

    // ============================================================
    // БАЗОВЫЕ ВИДЖЕТЫ
    // ============================================================

    m_qmlGenerators["QLabel"] = [](const QJsonObject& spec) {
        QString text = spec["text"].toString("Label");
        QString color = spec["properties"].toObject()["color"].toString("#ffffff");
        int fontSize = spec["properties"].toObject()["font.pixelSize"].toInt(14);
        bool bold = spec["properties"].toObject()["font.bold"].toBool(false);
        QString alignment = spec["properties"].toObject()["alignment"].toString("left");

        QString alignStr = "Text.AlignLeft";
        if (alignment == "center") alignStr = "Text.AlignHCenter";
        else if (alignment == "right") alignStr = "Text.AlignRight";

        return QString(
            "Text {\n"
            "    text: \"%1\"\n"
            "    color: \"%2\"\n"
            "    font.pixelSize: %3\n"
            "    font.bold: %4\n"
            "    horizontalAlignment: %5\n"
            "    wrapMode: Text.WordWrap\n"
            "    Layout.fillWidth: true\n"
            "    Layout.preferredHeight: implicitHeight\n"
            "}\n"
        ).arg(text).arg(color).arg(fontSize).arg(bold ? "true" : "false").arg(alignStr);
    };

    m_qmlGenerators["QPushButton"] = [](const QJsonObject& spec) {
        QString text = spec["text"].toString("Button");
        return QString(
            "Button {\n"
            "    text: \"%1\"\n"
            "    Layout.fillWidth: true\n"
            "    Layout.preferredHeight: 36\n"
            "}\n"
        ).arg(text);
    };

    m_qmlGenerators["QComboBox"] = [](const QJsonObject& spec) {
        QStringList items;
        if (spec.contains("items") && spec["items"].isArray()) {
            QJsonArray itemsArray = spec["items"].toArray();
            for (const QJsonValue& val : itemsArray) {
                if (val.isObject()) {
                    items << val.toObject()["label"].toString();
                } else {
                    items << val.toString();
                }
            }
        }
        if (items.isEmpty()) {
            items << "Option 1" << "Option 2" << "Option 3";
        }

        QString modelStr = "[\"" + items.join("\", \"") + "\"]";
        return QString(
            "ComboBox {\n"
            "    model: %1\n"
            "    Layout.fillWidth: true\n"
            "    Layout.preferredHeight: 36\n"
            "}\n"
        ).arg(modelStr);
    };

    m_qmlGenerators["QLineEdit"] = [](const QJsonObject& spec) {
        QString placeholder = spec["placeholder"].toString("Введите текст...");
        return QString(
            "TextField {\n"
            "    placeholderText: \"%1\"\n"
            "    Layout.fillWidth: true\n"
            "    Layout.preferredHeight: 36\n"
            "}\n"
        ).arg(placeholder);
    };

    m_qmlGenerators["QCheckBox"] = [](const QJsonObject& spec) {
        QString text = spec["text"].toString("CheckBox");
        bool checked = spec["checked"].toBool(false);
        return QString(
            "CheckBox {\n"
            "    text: \"%1\"\n"
            "    checked: %2\n"
            "    Layout.fillWidth: true\n"
            "    Layout.preferredHeight: implicitHeight\n"
            "}\n"
        ).arg(text).arg(checked ? "true" : "false");
    };

    m_qmlGenerators["QRadioButton"] = [](const QJsonObject& spec) {
        QString text = spec["text"].toString("RadioButton");
        bool checked = spec["checked"].toBool(false);
        return QString(
            "RadioButton {\n"
            "    text: \"%1\"\n"
            "    checked: %2\n"
            "    Layout.fillWidth: true\n"
            "    Layout.preferredHeight: implicitHeight\n"
            "}\n"
        ).arg(text).arg(checked ? "true" : "false");
    };

    m_qmlGenerators["QProgressBar"] = [](const QJsonObject& spec) {
        int value = spec["value"].toInt(50);
        int min = spec["minimum"].toInt(0);
        int max = spec["maximum"].toInt(100);
        return QString(
            "ProgressBar {\n"
            "    value: %1\n"
            "    from: %2\n"
            "    to: %3\n"
            "    Layout.fillWidth: true\n"
            "    Layout.preferredHeight: 20\n"
            "}\n"
        ).arg(value).arg(min).arg(max);
    };

    m_qmlGenerators["QDateEdit"] = [](const QJsonObject& spec) {
        QString date = spec["date"].toString("2026-08-16");
        return QString(
            "ComboBox {\n"
            "    model: [\"%1\"]\n"
            "    Layout.fillWidth: true\n"
            "    Layout.preferredHeight: 36\n"
            "    editable: true\n"
            "}\n"
        ).arg(date);
    };

    m_qmlGenerators["QDateTimeEdit"] = [](const QJsonObject& spec) {
        QString datetime = spec["datetime"].toString("2026-08-16 10:30");
        return QString(
            "ComboBox {\n"
            "    model: [\"%1\"]\n"
            "    Layout.fillWidth: true\n"
            "    Layout.preferredHeight: 36\n"
            "    editable: true\n"
            "}\n"
        ).arg(datetime);
    };

    m_qmlGenerators["QCalendarWidget"] = [](const QJsonObject& spec) {
        return QString(
            "Rectangle {\n"
            "    color: \"transparent\"\n"
            "    Layout.fillWidth: true\n"
            "    Layout.preferredHeight: 300\n"
            "    Text {\n"
            "        anchors.centerIn: parent\n"
            "        text: \"Календарь\"\n"
            "        color: \"#888\"\n"
            "        font.pixelSize: 16\n"
            "    }\n"
            "}\n"
        );
    };

    // ============================================================
    // QTableWidget — правильное количество колонок
    // ============================================================
    m_qmlGenerators["QTableWidget"] = [](const QJsonObject& spec) {
        // Парсим колонки из JSON
        QStringList columnLabels;
        QList<int> columnWidths;

        if (spec.contains("columns") && spec["columns"].isArray()) {
            QJsonArray columns = spec["columns"].toArray();
            for (const QJsonValue& colVal : columns) {
                if (colVal.isObject()) {
                    QJsonObject colObj = colVal.toObject();
                    columnLabels << colObj["label"].toString("Колонка");
                    columnWidths << colObj["width"].toInt(100);
                }
            }
        }

        if (columnLabels.isEmpty()) {
            columnLabels << "ID" << "Наименование" << "Статус";
            columnWidths << 50 << 200 << 120;
        }

        QString headerCode;
        for (int i = 0; i < columnLabels.size(); ++i) {
            headerCode += QString(
                "Rectangle {\n"
                "    width: %1\n"
                "    height: 30\n"
                "    color: \"#2a2a2a\"\n"
                "    border.color: \"#444\"\n"
                "    border.width: 1\n"
                "    Text {\n"
                "        anchors.centerIn: parent\n"
                "        text: \"%2\"\n"
                "        color: \"#aaa\"\n"
                "        font.pixelSize: 12\n"
                "        font.bold: true\n"
                "    }\n"
                "}\n"
            ).arg(columnWidths[i]).arg(columnLabels[i]);
        }

        // Генерируем данные в соответствии с количеством колонок
        QStringList rowData;
        if (columnLabels.size() == 3) {
            rowData = {
                "1|Заказ №1|Выполнен",
                "2|Заказ №2|В работе",
                "3|Заказ №3|Ожидает",
                "4|Заказ №4|Отменён",
                "5|Заказ №5|Выполнен"
            };
        } else if (columnLabels.size() == 4) {
            rowData = {
                "1|Документ А|1.0|Утверждён",
                "2|Документ Б|2.1|На проверке",
                "3|Документ В|0.9|Черновик",
                "4|Документ Г|1.2|Утверждён",
                "5|Документ Д|0.5|Черновик"
            };
        } else {
            // Для произвольного количества колонок
            rowData = {
                "1|Значение 1|Статус 1|Дополнительно",
                "2|Значение 2|Статус 2|Дополнительно",
                "3|Значение 3|Статус 3|Дополнительно"
            };
        }

        QString delegateCode;
        for (const QString& row : rowData) {
            QStringList cells = row.split("|");
            QString rowCode;
            for (int i = 0; i < columnLabels.size() && i < cells.size(); ++i) {
                rowCode += QString(
                    "Rectangle {\n"
                    "    width: %1\n"
                    "    height: 28\n"
                    "    color: index % 2 == 0 ? \"#1a1a1a\" : \"#222222\"\n"
                    "    border.color: \"#333\"\n"
                    "    border.width: 1\n"
                    "    Text {\n"
                    "        anchors.centerIn: parent\n"
                    "        text: \"%2\"\n"
                    "        color: \"#ccc\"\n"
                    "        font.pixelSize: 11\n"
                    "        elide: Text.ElideRight\n"
                    "    }\n"
                    "}\n"
                ).arg(columnWidths[i]).arg(cells[i]);
            }
            delegateCode += QString(
                "Row {\n"
                "    spacing: 0\n"
                "    %1\n"
                "}\n"
            ).arg(rowCode);
        }

        return QString(
            "Rectangle {\n"
            "    color: \"#121212\"\n"
            "    Layout.fillWidth: true\n"
            "    Layout.preferredHeight: 200\n"
            "    border.color: \"#444\"\n"
            "    border.width: 1\n"
            "    radius: 4\n"
            "    ColumnLayout {\n"
            "        anchors.fill: parent\n"
            "        spacing: 0\n"
            "        Row {\n"
            "            spacing: 0\n"
            "            %1\n"
            "        }\n"
            "        Rectangle { height: 1; Layout.fillWidth: true; color: \"#444\" }\n"
            "        ScrollView {\n"
            "            Layout.fillWidth: true\n"
            "            Layout.fillHeight: true\n"
            "            clip: true\n"
            "            ScrollBar.vertical.policy: ScrollBar.AsNeeded\n"
            "            Column {\n"
            "                spacing: 0\n"
            "                %2\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "}\n"
        ).arg(headerCode).arg(delegateCode);
    };

    // ============================================================
    // LAYOUT-КОНТЕЙНЕРЫ
    // ============================================================

    m_qmlGenerators["QVBoxLayout"] = [](const QJsonObject& spec) {
        return QString(
            "ColumnLayout {\n"
            "    Layout.fillWidth: true\n"
            "    spacing: 8\n"
            "}\n"
        );
    };

    m_qmlGenerators["QHBoxLayout"] = [](const QJsonObject& spec) {
        return QString(
            "RowLayout {\n"
            "    Layout.fillWidth: true\n"
            "    spacing: 10\n"
            "}\n"
        );
    };

    m_qmlGenerators["QGridLayout"] = [](const QJsonObject& spec) {
        return QString(
            "GridLayout {\n"
            "    Layout.fillWidth: true\n"
            "    flow: GridLayout.TopToBottom\n"
            "    columns: 2\n"
            "}\n"
        );
    };

    m_qmlGenerators["QGroupBox"] = [](const QJsonObject& spec) {
        QString title = spec["title"].toString("Group");
        return QString(
            "GroupBox {\n"
            "    title: \"%1\"\n"
            "    Layout.fillWidth: true\n"
            "    Layout.minimumHeight: 200\n"
            "    ColumnLayout {\n"
            "        anchors.fill: parent\n"
            "        spacing: 8\n"
            "    }\n"
            "}\n"
        ).arg(title);
    };

    // ============================================================
    // ДИАГРАММЫ
    // ============================================================

    // ChartPie
    m_qmlGenerators["ChartPie"] = [](const QJsonObject& spec) {
        QString title = spec["title"].toString("Диаграмма");
        int preferredHeight = spec["height"].toInt(350);
        if (preferredHeight <= 0) preferredHeight = 350;

        QJsonArray values;
        QStringList labels;
        parseChartData(spec, values, labels);

        QString valuesStr = "[";
        QString labelsWithValuesStr = "[";
        for (int i = 0; i < values.size(); ++i) {
            if (i > 0) {
                valuesStr += ", ";
                labelsWithValuesStr += ", ";
            }
            double val = values[i].toDouble();
            valuesStr += QString::number(val);
            labelsWithValuesStr += "\"" + labels[i] + " (" + QString::number(val) + ")\"";
        }
        valuesStr += "]";
        labelsWithValuesStr += "]";

        QString colorsStr =
            "[\"#ff6b6b\", \"#feca57\", \"#48dbfb\", \"#ff9ff3\", \"#54a0ff\", "
            "\"#1dd1a1\", \"#f368e0\", \"#00d2d3\", \"#ff9f43\", \"#a29bfe\"]";

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
            "                anchors.fill: parent\n"
            "                anchors.margins: 10\n"
            "                onPaint: {\n"
            "                    var ctx = getContext(\"2d\");\n"
            "                    var cx = width / 2;\n"
            "                    var cy = height / 2;\n"
            "                    var r = Math.min(width, height) / 2 - 20;\n"
            "                    if (r < 10) r = 10;\n"
            "                    var data = %3;\n"
            "                    var colors = %4;\n"
            "                    var total = data.reduce(function(a,b) { return a + b; }, 0);\n"
            "                    if (total === 0) total = 1;\n"
            "                    var startAngle = 0;\n"
            "                    for (var i = 0; i < data.length; i++) {\n"
            "                        var angle = (data[i] / total) * 2 * Math.PI;\n"
            "                        ctx.beginPath();\n"
            "                        ctx.moveTo(cx, cy);\n"
            "                        ctx.arc(cx, cy, r, startAngle, startAngle + angle);\n"
            "                        ctx.closePath();\n"
            "                        ctx.fillStyle = colors[i % colors.length];\n"
            "                        ctx.fill();\n"
            "                        startAngle += angle;\n"
            "                    }\n"
            "                    ctx.strokeStyle = \"#333\";\n"
            "                    ctx.lineWidth = 2;\n"
            "                    ctx.stroke();\n"
            "                }\n"
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
            "                        color: %4[index % %4.length]\n"
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
        ).arg(preferredHeight).arg(title).arg(valuesStr).arg(colorsStr).arg(labelsWithValuesStr);
    };

    // ChartBar
    m_qmlGenerators["ChartBar"] = [](const QJsonObject& spec) {
        QString title = spec["title"].toString("Диаграмма");
        int preferredHeight = spec["height"].toInt(350);
        if (preferredHeight <= 0) preferredHeight = 350;

        QJsonArray values;
        QStringList labels;
        parseChartData(spec, values, labels);

        QString valuesStr = "[";
        QString labelsWithValuesStr = "[";
        for (int i = 0; i < values.size(); ++i) {
            if (i > 0) {
                valuesStr += ", ";
                labelsWithValuesStr += ", ";
            }
            double val = values[i].toDouble();
            valuesStr += QString::number(val);
            labelsWithValuesStr += "\"" + labels[i] + " (" + QString::number(val) + ")\"";
        }
        valuesStr += "]";
        labelsWithValuesStr += "]";

        QString colorsStr =
            "[\"#ff6b6b\", \"#feca57\", \"#48dbfb\", \"#ff9ff3\", \"#54a0ff\", "
            "\"#1dd1a1\", \"#f368e0\", \"#00d2d3\", \"#ff9f43\", \"#a29bfe\"]";

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
            "                anchors.fill: parent\n"
            "                anchors.margins: 10\n"
            "                onPaint: {\n"
            "                    var ctx = getContext(\"2d\");\n"
            "                    var data = %3;\n"
            "                    var colors = %4;\n"
            "                    var maxVal = Math.max.apply(null, data);\n"
            "                    if (maxVal === 0) maxVal = 1;\n"
            "                    var barWidth = width / data.length * 0.7;\n"
            "                    var gap = (width / data.length - barWidth) / 2;\n"
            "                    for (var i = 0; i < data.length; i++) {\n"
            "                        var h = (data[i] / maxVal) * (height - 20);\n"
            "                        var x = i * (barWidth + gap * 2) + gap;\n"
            "                        var y = height - h - 10;\n"
            "                        ctx.fillStyle = colors[i % colors.length];\n"
            "                        ctx.fillRect(x, y, barWidth, h);\n"
            "                        ctx.strokeStyle = \"#333\";\n"
            "                        ctx.lineWidth = 1;\n"
            "                        ctx.strokeRect(x, y, barWidth, h);\n"
            "                    }\n"
            "                }\n"
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
            "                        color: %4[index % %4.length]\n"
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
        ).arg(preferredHeight).arg(title).arg(valuesStr).arg(colorsStr).arg(labelsWithValuesStr);
    };

    // ChartBarCompare
    m_qmlGenerators["ChartBarCompare"] = [](const QJsonObject& spec) {
        QString title = spec["title"].toString("Сравнение");
        int preferredHeight = spec["height"].toInt(350);
        if (preferredHeight <= 0) preferredHeight = 350;

        QJsonArray values1;
        QStringList labels1;
        parseChartData(spec, values1, labels1);

        QJsonArray values2;
        QStringList labels2;
        if (spec.contains("data2") && spec["data2"].isObject()) {
            QJsonObject data2Obj = spec["data2"].toObject();
            if (data2Obj.contains("Marked_values") && data2Obj["Marked_values"].isArray()) {
                QJsonArray markedValues = data2Obj["Marked_values"].toArray();
                for (const QJsonValue& item : markedValues) {
                    if (item.isObject()) {
                        QJsonObject obj = item.toObject();
                        if (obj.contains("value")) {
                            values2.append(obj["value"].toDouble());
                            if (obj.contains("label")) {
                                labels2 << obj["label"].toString();
                            } else {
                                labels2 << QString("Значение %1").arg(values2.size());
                            }
                        }
                    }
                }
            }
        }
        if (values2.isEmpty()) {
            values2 = {40, 70, 50, 80, 55};
            labels2 = {"Ряд 2-1", "Ряд 2-2", "Ряд 2-3", "Ряд 2-4", "Ряд 2-5"};
        }

        QString valuesStr1 = "[";
        for (int i = 0; i < values1.size(); ++i) {
            if (i > 0) valuesStr1 += ", ";
            valuesStr1 += QString::number(values1[i].toDouble());
        }
        valuesStr1 += "]";

        QString valuesStr2 = "[";
        for (int i = 0; i < values2.size(); ++i) {
            if (i > 0) valuesStr2 += ", ";
            valuesStr2 += QString::number(values2[i].toDouble());
        }
        valuesStr2 += "]";

        QString colorsStr1 =
            "[\"#ff6b6b\", \"#feca57\", \"#48dbfb\", \"#ff9ff3\", \"#54a0ff\"]";
        QString colorsStr2 =
            "[\"#ff9f43\", \"#ff6b6b\", \"#1dd1a1\", \"#f368e0\", \"#00d2d3\"]";

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
            "                anchors.fill: parent\n"
            "                anchors.margins: 10\n"
            "                onPaint: {\n"
            "                    var ctx = getContext(\"2d\");\n"
            "                    var data1 = %3;\n"
            "                    var data2 = %4;\n"
            "                    var colors1 = %5;\n"
            "                    var colors2 = %6;\n"
            "                    var maxVal = Math.max.apply(null, data1.concat(data2));\n"
            "                    if (maxVal === 0) maxVal = 1;\n"
            "                    var groupWidth = width / data1.length * 0.7;\n"
            "                    var barWidth = groupWidth / 2 - 2;\n"
            "                    var gap = (width / data1.length - groupWidth) / 2;\n"
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
        ).arg(preferredHeight).arg(title)
         .arg(valuesStr1).arg(valuesStr2)
         .arg(colorsStr1).arg(colorsStr2);
    };

    // ChartLine
    m_qmlGenerators["ChartLine"] = [](const QJsonObject& spec) {
        QString title = spec["title"].toString("График");
        int preferredHeight = spec["height"].toInt(350);
        if (preferredHeight <= 0) preferredHeight = 350;

        QJsonArray values;
        QStringList labels;
        parseChartData(spec, values, labels);

        QString valuesStr = "[";
        QString labelsWithValuesStr = "[";
        for (int i = 0; i < values.size(); ++i) {
            if (i > 0) {
                valuesStr += ", ";
                labelsWithValuesStr += ", ";
            }
            double val = values[i].toDouble();
            valuesStr += QString::number(val);
            labelsWithValuesStr += "\"" + labels[i] + " (" + QString::number(val) + ")\"";
        }
        valuesStr += "]";
        labelsWithValuesStr += "]";

        QString colorsStr =
            "[\"#48dbfb\", \"#ff6b6b\", \"#feca57\", \"#1dd1a1\", \"#ff9ff3\", "
            "\"#54a0ff\", \"#f368e0\", \"#00d2d3\", \"#ff9f43\", \"#a29bfe\"]";

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
            "                anchors.fill: parent\n"
            "                anchors.margins: 10\n"
            "                onPaint: {\n"
            "                    var ctx = getContext(\"2d\");\n"
            "                    var data = %3;\n"
            "                    var colors = %4;\n"
            "                    var maxVal = Math.max.apply(null, data);\n"
            "                    if (maxVal === 0) maxVal = 1;\n"
            "                    var stepX = width / (data.length - 1);\n"
            "                    ctx.beginPath();\n"
            "                    ctx.strokeStyle = colors[0];\n"
            "                    ctx.lineWidth = 2;\n"
            "                    for (var i = 0; i < data.length; i++) {\n"
            "                        var x = i * stepX;\n"
            "                        var y = height - (data[i] / maxVal) * (height - 20) - 10;\n"
            "                        if (i === 0) ctx.moveTo(x, y);\n"
            "                        else ctx.lineTo(x, y);\n"
            "                    }\n"
            "                    ctx.stroke();\n"
            "                    ctx.fillStyle = colors[0];\n"
            "                    for (var i = 0; i < data.length; i++) {\n"
            "                        var x = i * stepX;\n"
            "                        var y = height - (data[i] / maxVal) * (height - 20) - 10;\n"
            "                        ctx.beginPath();\n"
            "                        ctx.arc(x, y, 4, 0, 2 * Math.PI);\n"
            "                        ctx.fill();\n"
            "                    }\n"
            "                }\n"
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
            "                        color: %4[index % %4.length]\n"
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
        ).arg(preferredHeight).arg(title).arg(valuesStr).arg(colorsStr).arg(labelsWithValuesStr);
    };

    // ============================================================
    // ОСТАЛЬНЫЕ ВИДЖЕТЫ (заглушки)
    // ============================================================

    m_qmlGenerators["QListWidget"] = [](const QJsonObject& spec) {
        return QString(
            "Rectangle {\n"
            "    color: \"transparent\"\n"
            "    Layout.fillWidth: true\n"
            "    Layout.preferredHeight: 150\n"
            "    border.color: \"#444\"\n"
            "    border.width: 1\n"
            "    radius: 4\n"
            "    Text {\n"
            "        anchors.centerIn: parent\n"
            "        text: \"Список\"\n"
            "        color: \"#888\"\n"
            "        font.pixelSize: 14\n"
            "    }\n"
            "}\n"
        );
    };

    m_qmlGenerators["QTreeWidget"] = [](const QJsonObject& spec) {
        return QString(
            "Rectangle {\n"
            "    color: \"transparent\"\n"
            "    Layout.fillWidth: true\n"
            "    Layout.preferredHeight: 150\n"
            "    border.color: \"#444\"\n"
            "    border.width: 1\n"
            "    radius: 4\n"
            "    Text {\n"
            "        anchors.centerIn: parent\n"
            "        text: \"Дерево\"\n"
            "        color: \"#888\"\n"
            "        font.pixelSize: 14\n"
            "    }\n"
            "}\n"
        );
    };

    m_qmlGenerators["QFrame"] = [](const QJsonObject& spec) {
        return QString(
            "Rectangle {\n"
            "    height: 2\n"
            "    Layout.fillWidth: true\n"
            "    color: \"#444\"\n"
            "    Layout.margins: 5\n"
            "}\n"
        );
    };

    m_qmlGenerators["QSplitter"] = [](const QJsonObject& spec) {
        return QString(
            "SplitView {\n"
            "    Layout.fillWidth: true\n"
            "    Layout.fillHeight: true\n"
            "    orientation: Qt.Horizontal\n"
            "}\n"
        );
    };

    m_qmlGenerators["QToolBox"] = [](const QJsonObject& spec) {
        return QString(
            "TabBar {\n"
            "    Layout.fillWidth: true\n"
            "    TabButton { text: \"Вкладка 1\" }\n"
            "    TabButton { text: \"Вкладка 2\" }\n"
            "}\n"
        );
    };

    m_qmlGenerators["QTextEdit"] = [](const QJsonObject& spec) {
        return QString(
            "TextArea {\n"
            "    placeholderText: \"Введите текст...\"\n"
            "    Layout.fillWidth: true\n"
            "    Layout.preferredHeight: 100\n"
            "}\n"
        );
    };

    m_qmlGenerators["QPlainTextEdit"] = [](const QJsonObject& spec) {
        return QString(
            "TextArea {\n"
            "    placeholderText: \"Введите текст...\"\n"
            "    Layout.fillWidth: true\n"
            "    Layout.preferredHeight: 100\n"
            "}\n"
        );
    };

    m_qmlGenerators["QSlider"] = [](const QJsonObject& spec) {
        int min = spec["minimum"].toInt(0);
        int max = spec["maximum"].toInt(100);
        int value = spec["value"].toInt(50);
        return QString(
            "Slider {\n"
            "    from: %1\n"
            "    to: %2\n"
            "    value: %3\n"
            "    Layout.fillWidth: true\n"
            "}\n"
        ).arg(min).arg(max).arg(value);
    };

    m_qmlGenerators["QSpinBox"] = [](const QJsonObject& spec) {
        int min = spec["minimum"].toInt(0);
        int max = spec["maximum"].toInt(100);
        int value = spec["value"].toInt(50);
        return QString(
            "SpinBox {\n"
            "    from: %1\n"
            "    to: %2\n"
            "    value: %3\n"
            "    Layout.fillWidth: true\n"
            "}\n"
        ).arg(min).arg(max).arg(value);
    };

    m_qmlGenerators["QTabWidget"] = [](const QJsonObject& spec) {
        return QString(
            "ColumnLayout {\n"
            "    Layout.fillWidth: true\n"
            "    Layout.fillHeight: true\n"
            "    TabBar {\n"
            "        id: tabBar\n"
            "        Layout.fillWidth: true\n"
            "        TabButton { text: \"Вкладка 1\" }\n"
            "        TabButton { text: \"Вкладка 2\" }\n"
            "    }\n"
            "    StackLayout {\n"
            "        currentIndex: tabBar.currentIndex\n"
            "        Layout.fillWidth: true\n"
            "        Layout.fillHeight: true\n"
            "        Rectangle { color: \"transparent\"\n"
            "            Text { anchors.centerIn: parent; text: \"Содержимое 1\"; color: \"#888\" } }\n"
            "        Rectangle { color: \"transparent\"\n"
            "            Text { anchors.centerIn: parent; text: \"Содержимое 2\"; color: \"#888\" } }\n"
            "    }\n"
            "}\n"
        );
    };

    m_qmlGenerators["QStackedWidget"] = [](const QJsonObject& spec) {
        return QString(
            "StackLayout {\n"
            "    Layout.fillWidth: true\n"
            "    Layout.fillHeight: true\n"
            "    Rectangle { color: \"transparent\"\n"
            "        Text { anchors.centerIn: parent; text: \"Страница 1\"; color: \"#888\" } }\n"
            "    Rectangle { color: \"transparent\"\n"
            "        Text { anchors.centerIn: parent; text: \"Страница 2\"; color: \"#888\" } }\n"
            "}\n"
        );
    };

    m_qmlGenerators["QToolButton"] = [](const QJsonObject& spec) {
        QString text = spec["text"].toString("🔧");
        return QString(
            "Button {\n"
            "    text: \"%1\"\n"
            "    Layout.preferredWidth: 40\n"
            "    Layout.preferredHeight: 40\n"
            "    flat: true\n"
            "}\n"
        ).arg(text);
    };

    qDebug() << "  All widget types registered. Total:" << m_qmlGenerators.size();
}

QObject* QmlObjectFactory::create(const QString& type, const QJsonObject& spec, QQuickItem* parent)
{
    qDebug() << "    QmlObjectFactory::create(" << type << ") parent=" << (parent ? "valid" : "null")
             << "parent class:" << (parent ? parent->metaObject()->className() : "null");

    if (!m_qmlGenerators.contains(type)) {
        qWarning() << "      UNKNOWN widget type:" << type;
        return nullptr;
    }

    QString qmlCode = m_qmlGenerators[type](spec);

    qDebug() << "      QML code length:" << qmlCode.length() << "bytes";

    QString fullQml = QString(
        "import QtQuick 6.0\n"
        "import QtQuick.Controls 6.0\n"
        "import QtQuick.Layouts 6.0\n"
        "%1\n"
    ).arg(qmlCode);

    QQmlComponent component(m_engine);
    component.setData(fullQml.toUtf8(), QUrl());

    if (component.isError()) {
        qWarning() << "      QML component error:" << component.errorString();
        return nullptr;
    }

    QObject* object = component.create();
    if (!object) {
        qWarning() << "      Failed to create widget:" << component.errorString();
        return nullptr;
    }

    QQuickItem* item = qobject_cast<QQuickItem*>(object);
    if (item && parent) {
        item->setParentItem(parent);
        qDebug() << "      Widget parented to parentItem (setParentItem)";
    } else if (item) {
        qDebug() << "      Widget created WITHOUT parent";
    }

    qDebug() << "      Widget created SUCCESSFULLY";

    return object;
}
