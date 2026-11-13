import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0

Dialog
{
    id: root
    title: "Создать группу"
    width: 400
    height: 220
    modal: true
    standardButtons: Dialog.NoButton
    anchors.centerIn: Overlay.overlay

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 15

        Label { text: "Название группы:" }

        TextField
        {
            id: nameField
            Layout.fillWidth: true
            Layout.preferredHeight: 35
            placeholderText: "Введите название"
            onAccepted: if (text !== "") root.accept()
        }

        Item { Layout.fillHeight: true }

        RowLayout
        {
            Layout.alignment: Qt.AlignRight
            spacing: 10

            Button { text: "Отмена"; onClicked: root.close() }
            Button
            {
                text: "Создать"
                enabled: nameField.text !== ""
                onClicked: root.accept()
            }
        }
    }

    onOpened: nameField.text = ""

    onAccepted: {
        if (projectController && nameField.text !== "")
            projectController.createCustomGroup(nameField.text)
    }
}
