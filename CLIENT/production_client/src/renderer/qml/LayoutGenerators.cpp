#include <QString>
#include <QJsonObject>

QString generateVBoxLayout(const QJsonObject& spec)
{
    return QString(
        "ColumnLayout {\n"
        "    Layout.fillWidth: true\n"
        "    spacing: 8\n"
        "}\n"
    );
}

QString generateHBoxLayout(const QJsonObject& spec)
{
    return QString(
        "RowLayout {\n"
        "    Layout.fillWidth: true\n"
        "    spacing: 10\n"
        "}\n"
    );
}

QString generateGridLayout(const QJsonObject& spec)
{
    return QString(
        "GridLayout {\n"
        "    Layout.fillWidth: true\n"
        "    flow: GridLayout.TopToBottom\n"
        "    columns: 2\n"
        "}\n"
    );
}

QString generateGroupBox(const QJsonObject& spec)
{
    QString title = spec["title"].toString("Group");
    QString widgetId = spec["id"].toString();
    
    return QString(
        "GroupBox {\n"
        "    id: groupBox_%1\n"
        "    title: \"%2\"\n"
        "    Layout.fillWidth: true\n"
        "    Layout.topMargin: 8\n"
        "    Layout.bottomMargin: 8\n"
        "    padding: 16\n"
        "    spacing: 8\n"
        "    background: Rectangle {\n"
        "        color: \"transparent\"\n"
        "        border.color: Colors.border\n"
        "        border.width: 1\n"
        "        radius: 6\n"
        "    }\n"
        "    label: Text {\n"
        "        text: parent.title\n"
        "        color: Colors.text\n"
        "        font.pixelSize: 14\n"
        "        font.bold: true\n"
        "        padding: 4\n"
        "    }\n"
        "    ColumnLayout {\n"
        "        anchors.fill: parent\n"
        "        spacing: 8\n"
        "    }\n"
        "}\n"
    ).arg(widgetId).arg(title);
}
