#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QStringList>

QString generateTableWidget(const QJsonObject& spec)
{
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

    // Если колонки не заданы, используем стандартные
    if (columnLabels.isEmpty()) {
        columnLabels = {"ID", "Наименование", "Статус", "Срок"};
        columnWidths = {50, 200, 120, 120};
    }

    // Генерируем данные в соответствии с количеством колонок
    QStringList rowData;
    int colCount = columnLabels.size();
    
    if (colCount == 3) {
        rowData = {
            "1|Заказ №1|Выполнен",
            "2|Заказ №2|В работе",
            "3|Заказ №3|Ожидает",
            "4|Заказ №4|Отменён",
            "5|Заказ №5|Выполнен"
        };
    } else if (colCount == 4) {
        rowData = {
            "1|Документ А|1.0|Утверждён|2026-01-15",
            "2|Документ Б|2.1|На проверке|2026-02-01",
            "3|Документ В|0.9|Черновик|2026-03-10",
            "4|Документ Г|1.2|Утверждён|2026-04-20",
            "5|Документ Д|0.5|Черновик|2026-05-05"
        };
    } else if (colCount == 5) {
        rowData = {
            "1|Документ А|1.0|Утверждён|2026-01-15",
            "2|Документ Б|2.1|На проверке|2026-02-01",
            "3|Документ В|0.9|Черновик|2026-03-10",
            "4|Документ Г|1.2|Утверждён|2026-04-20",
            "5|Документ Д|0.5|Черновик|2026-05-05"
        };
    } else {
        // Для произвольного количества колонок
        QStringList cols;
        for (int i = 0; i < colCount; ++i) {
            cols << QString("Значение%1").arg(i+1);
        }
        rowData << cols.join("|");
        for (int i = 0; i < 3; ++i) {
            QStringList vals;
            for (int j = 0; j < colCount; ++j) {
                vals << QString("%1-%2").arg(i+1).arg(j+1);
            }
            rowData << vals.join("|");
        }
    }

    // Строим заголовок
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

    // Строим строки данных
    QString delegateCode;
    int rowIndex = 0;
    for (const QString& row : rowData) {
        QStringList cells = row.split("|");
        QString rowCode;
        for (int i = 0; i < columnLabels.size() && i < cells.size(); ++i) {
            rowCode += QString(
                "Rectangle {\n"
                "    width: %1\n"
                "    height: 28\n"
                "    color: %2 ? \"#1a1a1a\" : \"#222222\"\n"
                "    border.color: \"#333\"\n"
                "    border.width: 1\n"
                "    Text {\n"
                "        anchors.centerIn: parent\n"
                "        text: \"%3\"\n"
                "        color: \"#ccc\"\n"
                "        font.pixelSize: 11\n"
                "        elide: Text.ElideRight\n"
                "    }\n"
                "}\n"
            ).arg(columnWidths[i])
             .arg(rowIndex % 2 == 0 ? "true" : "false")
             .arg(cells[i]);
        }
        // Если в строке меньше ячеек, чем колонок, добавляем пустые
        for (int i = cells.size(); i < columnLabels.size(); ++i) {
            rowCode += QString(
                "Rectangle {\n"
                "    width: %1\n"
                "    height: 28\n"
                "    color: %2 ? \"#1a1a1a\" : \"#222222\"\n"
                "    border.color: \"#333\"\n"
                "    border.width: 1\n"
                "    Text {\n"
                "        anchors.centerIn: parent\n"
                "        text: \"\"\n"
                "        color: \"#ccc\"\n"
                "        font.pixelSize: 11\n"
                "    }\n"
                "}\n"
            ).arg(columnWidths[i])
             .arg(rowIndex % 2 == 0 ? "true" : "false");
        }
        delegateCode += QString(
            "Row {\n"
            "    spacing: 0\n"
            "    %1\n"
            "}\n"
        ).arg(rowCode);
        rowIndex++;
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
}
