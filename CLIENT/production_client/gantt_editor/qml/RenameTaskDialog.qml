import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0

Dialog
{
    id: root
    title: "Переименовать задачу"
    width: 400
    height: 230
    modal: true
    standardButtons: Dialog.NoButton
    anchors.centerIn: Overlay.overlay

    property string taskId: ""

    function openForTask(tId)
    {
        if (!tId || tId === "") return
        if (!projectController) return
        if (projectController.settingsManager.editingLocked) {
            showToast("Редактирование заблокировано")
            return
        }
        taskId = tId
        var task = projectController?.projectData?.taskModel?.getTask(tId)
        newNameField.text = task ? (task.title || "") : ""
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
        anchors.margins: 15
        spacing: 15

        Label
        {
            text: "Новое название задачи:"
            Layout.fillWidth: true
            font.pixelSize: 13
        }

        TextField
        {
            id: newNameField
            Layout.fillWidth: true
            Layout.preferredHeight: 35
            placeholderText: "Введите название задачи"
            font.pixelSize: 13
            focus: true
            onAccepted:
            {
                if (newNameField.text !== "") root.accept()
            }
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
        if (taskId === "" || !projectController || !projectController.projectData) return
        if (projectController.settingsManager.editingLocked) {
            showToast("Редактирование заблокировано")
            return
        }
        var task = projectController.projectData.taskModel.getTask(taskId)
        if (!task) return
        projectController.projectData.taskModel.updateTask(
            taskId,
            newNameField.text,
            task.responsible,
            task.startDate,
            task.endDate,
            task.status
        )
    }
}
