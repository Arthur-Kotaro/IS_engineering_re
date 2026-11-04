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
    standardButtons: Dialog.Ok | Dialog.Cancel
    anchors.centerIn: Overlay.overlay

    property string taskId: ""
    property string comment: ""

    function openForTask(tId)
    {
        if (!projectController) return
        if (projectController.settingsManager.editingLocked) {
            showToast("Редактирование заблокировано")
            return
        }
        taskId = tId
        var task = projectController?.projectData?.taskModel?.getTask(tId)
        comment = task ? (task.comment || "") : ""
        commentField.text = comment
        open()
    }

    function showToast(message)
    {
        var w = root.Window.window
        if (w && w.toast) w.toast.show(message)
        else if (typeof mainWindow !== "undefined" && mainWindow && mainWindow.toast) mainWindow.toast.show(message)
        else console.log("TOAST:", message)
    }

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 10

        Label { text: "Комментарий:"; font.bold: true }
        TextArea
        {
            id: commentField
            Layout.fillWidth: true
            Layout.fillHeight: true
            text: root.comment
            wrapMode: Text.WordWrap
        }
    }

    onAccepted:
    {
        if (taskId && projectController)
        {
            projectController.projectData.taskModel.setTaskComment(taskId, commentField.text)
            if (mainWindow && mainWindow.gridArea) mainWindow.gridArea.updateData()
        }
    }
}
