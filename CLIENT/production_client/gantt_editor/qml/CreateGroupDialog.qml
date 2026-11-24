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

    property string position: "bottom" // "top" или "bottom"
    property string afterGroupId: ""

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

    function openFor(position, afterGroupId) {
        root.position = position || "bottom"
        root.afterGroupId = afterGroupId || ""
        nameField.text = ""
        root.open()
    }

    onOpened: nameField.text = ""

    onAccepted: {
        if (projectController && nameField.text !== "") {
            if (root.position === "top") {
                projectController.createCustomGroupWithPosition(nameField.text, "top")
            } else {
                // Если задан afterGroupId, но у нас пока нет метода вставки после конкретной группы,
                // используем существующий метод (снизу)
                projectController.createCustomGroupWithPosition(nameField.text, "bottom")
            }
        }
    }
}
