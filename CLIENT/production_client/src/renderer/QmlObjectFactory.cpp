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
    registerBuiltInTypes();
}

void QmlObjectFactory::registerBuiltInTypes()
{
    // QLabel → Text
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

    // QPushButton → Button
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

    // QComboBox → ComboBox
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

    // QGroupBox → GroupBox с ColumnLayout внутри
    m_qmlGenerators["QGroupBox"] = [](const QJsonObject& spec) {
        QString title = spec["title"].toString("Group");
        return QString(
            "GroupBox {\n"
            "    title: \"%1\"\n"
            "    Layout.fillWidth: true\n"
            "    ColumnLayout {\n"
            "        anchors.fill: parent\n"
            "        spacing: 8\n"
            "    }\n"
            "}\n"
        ).arg(title);
    };

    // QHBoxLayout → RowLayout
    m_qmlGenerators["QHBoxLayout"] = [](const QJsonObject& spec) {
        return QString(
            "RowLayout {\n"
            "    Layout.fillWidth: true\n"
            "    spacing: 10\n"
            "}\n"
        );
    };

    // QLineEdit → TextField
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

    // QCheckBox → CheckBox
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

    // QRadioButton → RadioButton
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

    // QProgressBar → ProgressBar
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

    // QDateEdit → DatePicker
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

    // QDateTimeEdit → DatePicker
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

    // QCalendarWidget → Calendar
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

    // QTableWidget → TableView
    m_qmlGenerators["QTableWidget"] = [](const QJsonObject& spec) {
        return QString(
            "Rectangle {\n"
            "    color: \"transparent\"\n"
            "    Layout.fillWidth: true\n"
            "    Layout.preferredHeight: 200\n"
            "    border.color: \"#444\"\n"
            "    border.width: 1\n"
            "    radius: 4\n"
            "    Text {\n"
            "        anchors.centerIn: parent\n"
            "        text: \"Таблица\"\n"
            "        color: \"#888\"\n"
            "        font.pixelSize: 14\n"
            "    }\n"
            "}\n"
        );
    };

    // QListWidget → ListView
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

    // QTreeWidget → TreeView
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

    // QFrame → Rectangle
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

    // QSplitter → SplitView
    m_qmlGenerators["QSplitter"] = [](const QJsonObject& spec) {
        return QString(
            "SplitView {\n"
            "    Layout.fillWidth: true\n"
            "    Layout.fillHeight: true\n"
            "    orientation: Qt.Horizontal\n"
            "}\n"
        );
    };

    // QToolBox → TabBar
    m_qmlGenerators["QToolBox"] = [](const QJsonObject& spec) {
        return QString(
            "TabBar {\n"
            "    Layout.fillWidth: true\n"
            "    TabButton { text: \"Вкладка 1\" }\n"
            "    TabButton { text: \"Вкладка 2\" }\n"
            "}\n"
        );
    };

    // QTextEdit → TextArea
    m_qmlGenerators["QTextEdit"] = [](const QJsonObject& spec) {
        return QString(
            "TextArea {\n"
            "    placeholderText: \"Введите текст...\"\n"
            "    Layout.fillWidth: true\n"
            "    Layout.preferredHeight: 100\n"
            "}\n"
        );
    };

    // QPlainTextEdit → TextArea
    m_qmlGenerators["QPlainTextEdit"] = [](const QJsonObject& spec) {
        return QString(
            "TextArea {\n"
            "    placeholderText: \"Введите текст...\"\n"
            "    Layout.fillWidth: true\n"
            "    Layout.preferredHeight: 100\n"
            "}\n"
        );
    };

    // QSlider → Slider
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

    // QSpinBox → SpinBox
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

    // QTabWidget → TabBar + StackLayout
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

    // QStackedWidget → StackLayout
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

    // QToolButton → Button
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

    // ChartPie → Круговая диаграмма
    m_qmlGenerators["ChartPie"] = [](const QJsonObject& spec) {
        QString title = spec["title"].toString("Диаграмма");
        return QString(
            "Rectangle {\n"
            "    color: \"transparent\"\n"
            "    Layout.fillWidth: true\n"
            "    Layout.preferredHeight: 350\n"
            "    ColumnLayout {\n"
            "        anchors.fill: parent\n"
            "        Text {\n"
            "            text: \"%1\"\n"
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
            "                    var data = [30, 25, 20, 15, 10];\n"
            "                    var colors = [\"#ff6b6b\", \"#feca57\", \"#48dbfb\", \"#ff9ff3\", \"#54a0ff\"];\n"
            "                    var total = data.reduce((a,b) => a + b, 0);\n"
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
            "    }\n"
            "}\n"
        ).arg(title);
    };

    // ChartBar → Столбчатая диаграмма
    m_qmlGenerators["ChartBar"] = [](const QJsonObject& spec) {
        QString title = spec["title"].toString("Диаграмма");
        return QString(
            "Rectangle {\n"
            "    color: \"transparent\"\n"
            "    Layout.fillWidth: true\n"
            "    Layout.preferredHeight: 350\n"
            "    ColumnLayout {\n"
            "        anchors.fill: parent\n"
            "        Text {\n"
            "            text: \"%1\"\n"
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
            "                    var data = [50, 80, 30, 90, 60, 70, 40];\n"
            "                    var colors = [\"#ff6b6b\", \"#feca57\", \"#48dbfb\", \"#ff9ff3\", \"#54a0ff\", \"#1dd1a1\", \"#f368e0\"];\n"
            "                    var maxVal = Math.max(...data);\n"
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
            "    }\n"
            "}\n"
        ).arg(title);
    };

    // ChartBarCompare → Сравнительная столбчатая диаграмма
    m_qmlGenerators["ChartBarCompare"] = [](const QJsonObject& spec) {
        QString title = spec["title"].toString("Сравнение");
        return QString(
            "Rectangle {\n"
            "    color: \"transparent\"\n"
            "    Layout.fillWidth: true\n"
            "    Layout.preferredHeight: 350\n"
            "    ColumnLayout {\n"
            "        anchors.fill: parent\n"
            "        Text {\n"
            "            text: \"%1\"\n"
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
            "            Text {\n"
            "                anchors.centerIn: parent\n"
            "                text: \"Сравнительная диаграмма\"\n"
            "                color: \"#666\"\n"
            "                font.pixelSize: 14\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "}\n"
        ).arg(title);
    };

    // ChartLine → Линейный график
    m_qmlGenerators["ChartLine"] = [](const QJsonObject& spec) {
        QString title = spec["title"].toString("График");
        return QString(
            "Rectangle {\n"
            "    color: \"transparent\"\n"
            "    Layout.fillWidth: true\n"
            "    Layout.preferredHeight: 350\n"
            "    ColumnLayout {\n"
            "        anchors.fill: parent\n"
            "        Text {\n"
            "            text: \"%1\"\n"
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
            "                    var data = [10, 30, 25, 50, 40, 70, 60, 90, 80];\n"
            "                    var maxVal = Math.max(...data);\n"
            "                    var stepX = width / (data.length - 1);\n"
            "                    ctx.beginPath();\n"
            "                    ctx.strokeStyle = \"#48dbfb\";\n"
            "                    ctx.lineWidth = 2;\n"
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
            "                        ctx.arc(x, y, 4, 0, 2 * Math.PI);\n"
            "                        ctx.fill();\n"
            "                    }\n"
            "                }\n"
            "            }\n"
            "        }\n"
            "    }\n"
            "}\n"
        ).arg(title);
    };
}

QObject* QmlObjectFactory::create(const QString& type, const QJsonObject& spec, QQuickItem* parent)
{
    if (!m_qmlGenerators.contains(type)) {
        qWarning() << "Unknown widget type:" << type;
        return nullptr;
    }

    QString qmlCode = m_qmlGenerators[type](spec);

    // Для QHBoxLayout и QGroupBox не добавляем внешнюю обёртку
    // Они сами являются контейнерами
    QString fullQml;
    if (type == "QHBoxLayout" || type == "QGroupBox") {
        fullQml = QString(
            "import QtQuick 6.0\n"
            "import QtQuick.Controls 6.0\n"
            "import QtQuick.Layouts 6.0\n"
            "%1\n"
        ).arg(qmlCode);
    } else {
        // Для обычных виджетов обёртка не нужна — они сами встанут в Layout
        fullQml = QString(
            "import QtQuick 6.0\n"
            "import QtQuick.Controls 6.0\n"
            "import QtQuick.Layouts 6.0\n"
            "%1\n"
        ).arg(qmlCode);
    }

    QQmlComponent component(m_engine);
    component.setData(fullQml.toUtf8(), QUrl());

    if (component.isError()) {
        qWarning() << "QML component error:" << component.errorString();
        return nullptr;
    }

    QObject* object = component.create();
    if (!object) {
        qWarning() << "Failed to create widget:" << component.errorString();
        return nullptr;
    }

    QQuickItem* item = qobject_cast<QQuickItem*>(object);
    if (item && parent) {
        item->setParentItem(parent);
    }

    return object;
}
