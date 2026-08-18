#include <QString>
#include <QJsonObject>
#include <QJsonArray>

QString generateLabel(const QJsonObject& spec)
{
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
}

QString generatePushButton(const QJsonObject& spec)
{
    QString text = spec["text"].toString("Button");
    QString widgetId = spec["id"].toString();
    
    return QString(
        "Button {\n"
        "    text: \"%1\"\n"
        "    Layout.fillWidth: true\n"
        "    Layout.preferredHeight: 36\n"
        "    onClicked: {\n"
        "        if (widgetBridge) {\n"
        "            var input = {\n"
        "                'type': 'button',\n"
        "                'value': text,\n"
        "                'paramName': '%2'\n"
        "            }\n"
        "            widgetBridge.sendWidgetInput('%2', input)\n"
        "        }\n"
        "    }\n"
        "}\n"
    ).arg(text).arg(widgetId);
}

QString generateComboBox(const QJsonObject& spec)
{
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

    QString widgetId = spec["id"].toString();
    QString modelStr = "[\"" + items.join("\", \"") + "\"]";
    
    return QString(
        "ComboBox {\n"
        "    id: combo_%1\n"
        "    model: %2\n"
        "    Layout.fillWidth: true\n"
        "    Layout.preferredHeight: 36\n"
        "    onCurrentTextChanged: {\n"
        "        if (widgetBridge) {\n"
        "            var input = {\n"
        "                'type': 'select',\n"
        "                'value': currentText,\n"
        "                'paramName': '%1'\n"
        "            }\n"
        "            widgetBridge.sendWidgetInput('%1', input)\n"
        "        }\n"
        "    }\n"
        "}\n"
    ).arg(widgetId).arg(modelStr);
}

QString generateLineEdit(const QJsonObject& spec)
{
    QString placeholder = spec["placeholder"].toString("Введите текст...");
    QString widgetId = spec["id"].toString();
    
    return QString(
        "TextField {\n"
        "    id: lineEdit_%1\n"
        "    placeholderText: \"%2\"\n"
        "    Layout.fillWidth: true\n"
        "    Layout.preferredHeight: 36\n"
        "    onTextChanged: {\n"
        "        if (widgetBridge) {\n"
        "            var input = {\n"
        "                'type': 'text',\n"
        "                'value': text,\n"
        "                'paramName': '%1'\n"
        "            }\n"
        "            widgetBridge.sendWidgetInput('%1', input)\n"
        "        }\n"
        "    }\n"
        "}\n"
    ).arg(widgetId).arg(placeholder);
}

QString generateCheckBox(const QJsonObject& spec)
{
    QString text = spec["text"].toString("CheckBox");
    bool checked = spec["checked"].toBool(false);
    QString widgetId = spec["id"].toString();
    
    return QString(
        "CheckBox {\n"
        "    text: \"%1\"\n"
        "    checked: %2\n"
        "    Layout.fillWidth: true\n"
        "    Layout.preferredHeight: implicitHeight\n"
        "    onCheckedChanged: {\n"
        "        if (widgetBridge) {\n"
        "            var input = {\n"
        "                'type': 'checkbox',\n"
        "                'value': checked,\n"
        "                'paramName': '%3'\n"
        "            }\n"
        "            widgetBridge.sendWidgetInput('%3', input)\n"
        "        }\n"
        "    }\n"
        "}\n"
    ).arg(text).arg(checked ? "true" : "false").arg(widgetId);
}

QString generateRadioButton(const QJsonObject& spec)
{
    QString text = spec["text"].toString("RadioButton");
    bool checked = spec["checked"].toBool(false);
    QString widgetId = spec["id"].toString();
    
    return QString(
        "RadioButton {\n"
        "    text: \"%1\"\n"
        "    checked: %2\n"
        "    Layout.fillWidth: true\n"
        "    Layout.preferredHeight: implicitHeight\n"
        "    onCheckedChanged: {\n"
        "        if (widgetBridge && checked) {\n"
        "            var input = {\n"
        "                'type': 'radio',\n"
        "                'value': text,\n"
        "                'paramName': '%3'\n"
        "            }\n"
        "            widgetBridge.sendWidgetInput('%3', input)\n"
        "        }\n"
        "    }\n"
        "}\n"
    ).arg(text).arg(checked ? "true" : "false").arg(widgetId);
}

QString generateProgressBar(const QJsonObject& spec)
{
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
}

QString generateDateEdit(const QJsonObject& spec)
{
    QString date = spec["date"].toString("2026-08-16");
    QString widgetId = spec["id"].toString();
    
    return QString(
        "ComboBox {\n"
        "    id: date_%1\n"
        "    model: [\"%2\"]\n"
        "    Layout.fillWidth: true\n"
        "    Layout.preferredHeight: 36\n"
        "    editable: true\n"
        "    onCurrentTextChanged: {\n"
        "        if (widgetBridge) {\n"
        "            var input = {\n"
        "                'type': 'date',\n"
        "                'value': currentText,\n"
        "                'paramName': '%1'\n"
        "            }\n"
        "            widgetBridge.sendWidgetInput('%1', input)\n"
        "        }\n"
        "    }\n"
        "}\n"
    ).arg(widgetId).arg(date);
}

QString generateDateTimeEdit(const QJsonObject& spec)
{
    QString datetime = spec["datetime"].toString("2026-08-16 10:30");
    QString widgetId = spec["id"].toString();
    
    return QString(
        "ComboBox {\n"
        "    id: datetime_%1\n"
        "    model: [\"%2\"]\n"
        "    Layout.fillWidth: true\n"
        "    Layout.preferredHeight: 36\n"
        "    editable: true\n"
        "    onCurrentTextChanged: {\n"
        "        if (widgetBridge) {\n"
        "            var input = {\n"
        "                'type': 'datetime',\n"
        "                'value': currentText,\n"
        "                'paramName': '%1'\n"
        "            }\n"
        "            widgetBridge.sendWidgetInput('%1', input)\n"
        "        }\n"
        "    }\n"
        "}\n"
    ).arg(widgetId).arg(datetime);
}

QString generateCalendarWidget(const QJsonObject& spec)
{
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
}

QString generateTextEdit(const QJsonObject& spec)
{
    QString widgetId = spec["id"].toString();
    
    return QString(
        "TextArea {\n"
        "    id: textArea_%1\n"
        "    placeholderText: \"Введите текст...\"\n"
        "    Layout.fillWidth: true\n"
        "    Layout.preferredHeight: 100\n"
        "    onTextChanged: {\n"
        "        if (widgetBridge) {\n"
        "            var input = {\n"
        "                'type': 'textarea',\n"
        "                'value': text,\n"
        "                'paramName': '%1'\n"
        "            }\n"
        "            widgetBridge.sendWidgetInput('%1', input)\n"
        "        }\n"
        "    }\n"
        "}\n"
    ).arg(widgetId);
}

QString generateSlider(const QJsonObject& spec)
{
    int min = spec["minimum"].toInt(0);
    int max = spec["maximum"].toInt(100);
    int value = spec["value"].toInt(50);
    QString widgetId = spec["id"].toString();
    
    return QString(
        "Slider {\n"
        "    id: slider_%1\n"
        "    from: %2\n"
        "    to: %3\n"
        "    value: %4\n"
        "    Layout.fillWidth: true\n"
        "    onValueChanged: {\n"
        "        if (widgetBridge) {\n"
        "            var input = {\n"
        "                'type': 'slider',\n"
        "                'value': value,\n"
        "                'paramName': '%1'\n"
        "            }\n"
        "            widgetBridge.sendWidgetInput('%1', input)\n"
        "        }\n"
        "    }\n"
        "}\n"
    ).arg(widgetId).arg(min).arg(max).arg(value);
}

QString generateSpinBox(const QJsonObject& spec)
{
    int min = spec["minimum"].toInt(0);
    int max = spec["maximum"].toInt(100);
    int value = spec["value"].toInt(50);
    QString widgetId = spec["id"].toString();
    
    return QString(
        "SpinBox {\n"
        "    id: spin_%1\n"
        "    from: %2\n"
        "    to: %3\n"
        "    value: %4\n"
        "    Layout.fillWidth: true\n"
        "    onValueChanged: {\n"
        "        if (widgetBridge) {\n"
        "            var input = {\n"
        "                'type': 'number',\n"
        "                'value': value,\n"
        "                'paramName': '%1'\n"
        "            }\n"
        "            widgetBridge.sendWidgetInput('%1', input)\n"
        "        }\n"
        "    }\n"
        "}\n"
    ).arg(widgetId).arg(min).arg(max).arg(value);
}

// Заглушки для сложных виджетов (без ввода)
QString generateListWidget(const QJsonObject& spec)
{
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
}

QString generateTreeWidget(const QJsonObject& spec)
{
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
}

QString generateFrame(const QJsonObject& spec)
{
    return QString(
        "Rectangle {\n"
        "    height: 2\n"
        "    Layout.fillWidth: true\n"
        "    color: \"#444\"\n"
        "    Layout.margins: 5\n"
        "}\n"
    );
}

QString generateSplitter(const QJsonObject& spec)
{
    return QString(
        "SplitView {\n"
        "    Layout.fillWidth: true\n"
        "    Layout.fillHeight: true\n"
        "    orientation: Qt.Horizontal\n"
        "}\n"
    );
}

QString generateToolBox(const QJsonObject& spec)
{
    return QString(
        "TabBar {\n"
        "    Layout.fillWidth: true\n"
        "    TabButton { text: \"Вкладка 1\" }\n"
        "    TabButton { text: \"Вкладка 2\" }\n"
        "}\n"
    );
}

QString generateTabWidget(const QJsonObject& spec)
{
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
}

QString generateStackedWidget(const QJsonObject& spec)
{
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
}

QString generateToolButton(const QJsonObject& spec)
{
    QString text = spec["text"].toString("🔧");
    QString widgetId = spec["id"].toString();
    
    return QString(
        "Button {\n"
        "    text: \"%1\"\n"
        "    Layout.preferredWidth: 40\n"
        "    Layout.preferredHeight: 40\n"
        "    flat: true\n"
        "    onClicked: {\n"
        "        if (widgetBridge) {\n"
        "            var input = {\n"
        "                'type': 'button',\n"
        "                'value': text,\n"
        "                'paramName': '%2'\n"
        "            }\n"
        "            widgetBridge.sendWidgetInput('%2', input)\n"
        "        }\n"
        "    }\n"
        "}\n"
    ).arg(text).arg(widgetId);
}
