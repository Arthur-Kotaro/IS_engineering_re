import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0

Dialog
{
    id: root
    title: "Завершить задачу"
    width: 450
    height: 260
    modal: true
    standardButtons: Dialog.NoButton
    anchors.centerIn: Overlay.overlay

    property string taskId: ""
    property string reason: ""

    function openForTask(tId) {
        root.taskId = tId
        root.reason = ""
        reasonField.text = ""
        root.open()
    }

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 12

        Label
        {
            text: "Все задачи ЛГ завершены. Завершить задачу в мастерграфике?"
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            font.pixelSize: 13
        }

        Label
        {
            text: "После завершения прогнозный срок будет зафиксирован, статус МГ-задачи станет «Выполнено»."
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            font.pixelSize: 11
            color: "#666666"
        }

        Label { text: "Причина (необязательно):"; font.pixelSize: 12 }
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

            Button
            {
                text: "Отмена"
                onClicked: root.reject()
            }

            Button
            {
                text: "Завершить"
                onClicked: {
                    root.reason = reasonField.text
                    root.accept()
                }
            }
        }
    }

    onAccepted: {
        if (projectController && projectController.localGraphController)
            projectController.localGraphController.completeLocalGraph(root.reason)
        root.close()
    }

    onRejected: root.close()
}
