import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0
import Qt.labs.platform 1.1 as Labs

Dialog
{
    id: root
    title: "Настройки"
    width: 750
    height: 480
    modal: true
    standardButtons: Dialog.NoButton

    property string typologiesPath: projectController.settingsManager.resourcesPath
    property string taskGroupsPath: projectController.settingsManager.resourcesPath
    property string holidaysPath: projectController.settingsManager.holidaysPath

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 14

        Label
        {
            text: "Путь к файлу типологий проектов:"
            font.bold: true
            font.pixelSize: 14
        }

        RowLayout
        {
            Layout.fillWidth: true
            spacing: 10

            Rectangle
            {
                Layout.fillWidth: true
                height: 40
                border.color: "#999999"
                border.width: 1
                radius: 4
                color: "#ffffff"

                TextInput
                {
                    id: typologiesField
                    anchors.fill: parent
                    anchors.margins: 8
                    text: root.typologiesPath
                    font.pixelSize: 14
                    clip: true
                    verticalAlignment: Text.AlignVCenter
                    onTextChanged: root.typologiesPath = text
                }
            }

            Button
            {
                text: "Обзор..."
                Layout.preferredHeight: 40
                font.pixelSize: 14
                onClicked: folderDialogTypologies.open()
            }
        }

        Label
        {
            text: "Путь к файлу типовых групп задач:"
            font.bold: true
            font.pixelSize: 14
        }

        RowLayout
        {
            Layout.fillWidth: true
            spacing: 10

            Rectangle
            {
                Layout.fillWidth: true
                height: 40
                border.color: "#999999"
                border.width: 1
                radius: 4
                color: "#ffffff"

                TextInput
                {
                    id: taskGroupsField
                    anchors.fill: parent
                    anchors.margins: 8
                    text: root.taskGroupsPath
                    font.pixelSize: 14
                    clip: true
                    verticalAlignment: Text.AlignVCenter
                    onTextChanged: root.taskGroupsPath = text
                }
            }

            Button
            {
                text: "Обзор..."
                Layout.preferredHeight: 40
                font.pixelSize: 14
                onClicked: folderDialogTaskGroups.open()
            }
        }

        Label
        {
            text: "Путь к файлу праздников:"
            font.bold: true
            font.pixelSize: 14
        }

        RowLayout
        {
            Layout.fillWidth: true
            spacing: 10

            Rectangle
            {
                Layout.fillWidth: true
                height: 40
                border.color: "#999999"
                border.width: 1
                radius: 4
                color: "#ffffff"

                TextInput
                {
                    id: holidaysField
                    anchors.fill: parent
                    anchors.margins: 8
                    text: root.holidaysPath
                    font.pixelSize: 14
                    clip: true
                    verticalAlignment: Text.AlignVCenter
                    onTextChanged: root.holidaysPath = text
                }
            }

            Button
            {
                text: "Обзор..."
                Layout.preferredHeight: 40
                font.pixelSize: 14
                onClicked: folderDialogHolidays.open()
            }
        }

        Item { Layout.fillHeight: true }

        Label
        {
            text: "Примечание: ресурсные файлы загружаются из указанных директорий"
            wrapMode: Text.WordWrap
            font.pixelSize: 12
            color: "#666666"
            Layout.fillWidth: true
        }

        RowLayout
        {
            Layout.alignment: Qt.AlignRight
            spacing: 10

            Button
            {
                text: "OK"
                Layout.preferredHeight: 38
                Layout.preferredWidth: 100
                font.pixelSize: 14
                onClicked:
                {
                    projectController.settingsManager.resourcesPath = root.typologiesPath
                    projectController.resourceManager.resourcesPath = root.typologiesPath
                    projectController.settingsManager.holidaysPath = root.holidaysPath
                    root.close()
                }
            }

            Button
            {
                text: "Отмена"
                Layout.preferredHeight: 38
                Layout.preferredWidth: 100
                font.pixelSize: 14
                onClicked: root.close()
            }
        }
    }

    Labs.FolderDialog
    {
        id: folderDialogTypologies
        onAccepted: typologiesField.text = folder
    }

    Labs.FolderDialog
    {
        id: folderDialogTaskGroups
        onAccepted: taskGroupsField.text = folder
    }

    Labs.FolderDialog
    {
        id: folderDialogHolidays
        onAccepted: holidaysField.text = folder
    }

    onOpened:
    {
        root.typologiesPath = projectController.settingsManager.resourcesPath
        root.taskGroupsPath = projectController.settingsManager.resourcesPath
        root.holidaysPath = projectController.settingsManager.holidaysPath
    }
}
