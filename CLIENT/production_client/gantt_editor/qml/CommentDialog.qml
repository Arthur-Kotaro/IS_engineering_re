import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0

Dialog
{
    id: root
    title: "Комментарий к задаче"
    width: 400
    height: 250
    modal: true
    standardButtons: Dialog.NoButton
    anchors.centerIn: Overlay.overlay

    property string taskId: ""
    property string comment: ""

    function openForTask(tId) {
        if (!projectController) return
        if (projectController.settingsManager.editingLocked) {
            if (mainWindow && mainWindow.toast) mainWindow.toast.show("Редактирование заблокировано")
            return
        }
        taskId = tId
        var task = projectController?.projectData?.taskModel?.getTask(tId)
        comment = task ? (task.comment || "") : ""
        commentField.text = comment
        open()
    }

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 10

        Label { text: "Комментарий:"; font.bold: true }
        TextArea { id: commentField; Layout.fillWidth: true; Layout.fillHeight: true
            text: root.comment; wrapMode: Text.WordWrap }

        RowLayout
        {
            Layout.fillWidth: true; spacing: 10
            Item { Layout.fillWidth: true }
            Button { text: "OK"; Layout.preferredWidth: 100; Layout.preferredHeight: 34
                onClicked: {
                    if (taskId && projectController) {
                        projectController.updateTaskComment(taskId, commentField.text)
                        root.close()
                    }
                } }
            Button { text: "Отмена"; Layout.preferredWidth: 100; Layout.preferredHeight: 34
                onClicked: root.close() }
        }
    }
}
