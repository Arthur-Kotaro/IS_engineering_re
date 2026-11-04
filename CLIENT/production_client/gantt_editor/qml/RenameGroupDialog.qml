import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0

Dialog
{
    id: root
    title: "Переименовать группу"
    width: 400
    height: 230
    modal: true
    standardButtons: Dialog.NoButton
    anchors.centerIn: Overlay.overlay

    property string groupId: ""

    function openWithGroup(gId, gName)
    {
        root.groupId = gId
        newNameField.text = gName
        root.open()
    }

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 15

        Label { text: "Новое название группы:"; Layout.fillWidth: true; font.pixelSize: 13 }

        TextField
        {
            id: newNameField
            Layout.fillWidth: true
            Layout.preferredHeight: 35
            placeholderText: "Введите название группы"
            font.pixelSize: 13
            focus: true
            onAccepted: if (newNameField.text !== "") root.accept()
        }

        Item { Layout.fillHeight: true }

        RowLayout
        {
            Layout.fillWidth: true
            spacing: 10

            Button
            {
                text: "Отмена"
                Layout.fillWidth: true
                Layout.preferredHeight: 35
                onClicked: root.close()
            }

            Button
            {
                text: "ОК"
                Layout.fillWidth: true
                Layout.preferredHeight: 35
                enabled: newNameField.text !== ""
                onClicked: root.accept()
            }
        }
    }

    onAccepted:
    {
        if (newNameField.text !== "" && projectController && projectController.projectData && root.groupId)
            projectController.projectData.groupModel.renameGroup(root.groupId, newNameField.text)
    }
}
