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

void QmlObjectFactory::registerBuiltInTypes()
{
    qDebug() << "  Registering built-in widget types...";

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

    // QDateEdit → ComboBox (временная заглушка)
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

    // QDateTimeEdit → ComboBox (временная заглушка)
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

    // QCalendarWidget → Rectangle (заглушка)
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

    // QTableWidget → Rectangle (заглушка)
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

    // QListWidget → Rectangle (заглушка)
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

    // QTreeWidget → Rectangle (заглушка)
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

    // QFrame → Rectangle (разделитель)
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

    // QToolBox → TabBar (заглушка)
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

    // ============================================================
    // LAYOUT-КОНТЕЙНЕРЫ
    // ============================================================

    // QVBoxLayout → ColumnLayout
    m_qmlGenerators["QVBoxLayout"] = [](const QJsonObject& spec) {
        return QString(
            "ColumnLayout {\n"
            "    Layout.fillWidth: true\n"
            "    spacing: 8\n"
            "}\n"
        );
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

    // QGridLayout → GridLayout (пока заглушка)
    m_qmlGenerators["QGridLayout"] = [](const QJsonObject& spec) {
        return QString(
            "GridLayout {\n"
            "    Layout.fillWidth: true\n"
            "    flow: GridLayout.TopToBottom\n"
            "    columns: 2\n"
            "}\n"
        );
    };

    // QGroupBox
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
    // ГРАФИКИ (цветные прямоугольники)
    // ============================================================

    // ChartPie
    m_qmlGenerators["ChartPie"] = [](const QJsonObject& spec) {
        QString title = spec["title"].toString("Диаграмма");
        int preferredHeight = spec["height"].toInt(300);
        if (preferredHeight <= 0) preferredHeight = 300;

        qDebug() << "    GENERATING ChartPie:" << title << "height=" << preferredHeight;

        return QString(
            "Rectangle {\n"
            "    color: \"#e74c3c\"\n"
            "    Layout.fillWidth: true\n"
            "    Layout.preferredHeight: %1\n"
            "    border.color: \"#c0392b\"\n"
            "    border.width: 2\n"
            "    Text {\n"
            "        anchors.centerIn: parent\n"
            "        text: \"ChartPie: %2\\nheight=%1\"\n"
            "        color: \"#ffffff\"\n"
            "        font.pixelSize: 14\n"
            "        font.bold: true\n"
            "        horizontalAlignment: Text.AlignHCenter\n"
            "    }\n"
            "}\n"
        ).arg(preferredHeight).arg(title);
    };

    // ChartBar
    m_qmlGenerators["ChartBar"] = [](const QJsonObject& spec) {
        QString title = spec["title"].toString("Диаграмма");
        int preferredHeight = spec["height"].toInt(300);
        if (preferredHeight <= 0) preferredHeight = 300;

        qDebug() << "    GENERATING ChartBar:" << title << "height=" << preferredHeight;

        return QString(
            "Rectangle {\n"
            "    color: \"#3498db\"\n"
            "    Layout.fillWidth: true\n"
            "    Layout.preferredHeight: %1\n"
            "    border.color: \"#2980b9\"\n"
            "    border.width: 2\n"
            "    Text {\n"
            "        anchors.centerIn: parent\n"
            "        text: \"ChartBar: %2\\nheight=%1\"\n"
            "        color: \"#ffffff\"\n"
            "        font.pixelSize: 14\n"
            "        font.bold: true\n"
            "        horizontalAlignment: Text.AlignHCenter\n"
            "    }\n"
            "}\n"
        ).arg(preferredHeight).arg(title);
    };

    // ChartBarCompare
    m_qmlGenerators["ChartBarCompare"] = [](const QJsonObject& spec) {
        QString title = spec["title"].toString("Сравнение");
        int preferredHeight = spec["height"].toInt(300);
        if (preferredHeight <= 0) preferredHeight = 300;

        qDebug() << "    GENERATING ChartBarCompare:" << title << "height=" << preferredHeight;

        return QString(
            "Rectangle {\n"
            "    color: \"#2ecc71\"\n"
            "    Layout.fillWidth: true\n"
            "    Layout.preferredHeight: %1\n"
            "    border.color: \"#27ae60\"\n"
            "    border.width: 2\n"
            "    Text {\n"
            "        anchors.centerIn: parent\n"
            "        text: \"ChartBarCompare: %2\\nheight=%1\"\n"
            "        color: \"#ffffff\"\n"
            "        font.pixelSize: 14\n"
            "        font.bold: true\n"
            "        horizontalAlignment: Text.AlignHCenter\n"
            "    }\n"
            "}\n"
        ).arg(preferredHeight).arg(title);
    };

    // ChartLine
    m_qmlGenerators["ChartLine"] = [](const QJsonObject& spec) {
        QString title = spec["title"].toString("График");
        int preferredHeight = spec["height"].toInt(300);
        if (preferredHeight <= 0) preferredHeight = 300;

        qDebug() << "    GENERATING ChartLine:" << title << "height=" << preferredHeight;

        return QString(
            "Rectangle {\n"
            "    color: \"#f39c12\"\n"
            "    Layout.fillWidth: true\n"
            "    Layout.preferredHeight: %1\n"
            "    border.color: \"#e67e22\"\n"
            "    border.width: 2\n"
            "    Text {\n"
            "        anchors.centerIn: parent\n"
            "        text: \"ChartLine: %2\\nheight=%1\"\n"
            "        color: \"#ffffff\"\n"
            "        font.pixelSize: 14\n"
            "        font.bold: true\n"
            "        horizontalAlignment: Text.AlignHCenter\n"
            "    }\n"
            "}\n"
        ).arg(preferredHeight).arg(title);
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

    // Собираем полный QML-код без внешней обёртки
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
