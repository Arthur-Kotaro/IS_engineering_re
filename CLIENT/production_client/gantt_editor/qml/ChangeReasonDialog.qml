import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0

Dialog
{
    id: root
    title: "Причина изменения"
    width: 500
    height: 260
    modal: true
    standardButtons: Dialog.NoButton
    anchors.centerIn: Overlay.overlay

    property string reason: ""
    property var onConfirm: null

    function openWith(callback) {
        root.reason = ""
        root.onConfirm = callback
        reasonField.text = ""
        root.open()
    }

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 10

        Label
        {
            text: "Укажите причину изменения (обязательно в режиме Чистовик):"
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }

        TextArea
        {
            id: reasonField
            Layout.fillWidth: true
            Layout.fillHeight: true
            placeholderText: "Введите причину..."
            wrapMode: Text.WordWrap
        }

        RowLayout
        {
            Layout.alignment: Qt.AlignRight
            spacing: 10

            Button { text: "Отмена"; onClicked: root.reject() }

            Button
            {
                text: "OK"
                enabled: reasonField.text.length > 0
                onClicked: {
                    root.reason = reasonField.text
                    root.accept()
                }
            }
        }
    }

    onAccepted: {
        if (root.onConfirm) root.onConfirm(root.reason)
        root.close()
    }

    onRejected: root.close()
}
