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
}
