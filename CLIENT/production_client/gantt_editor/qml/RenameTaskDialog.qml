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

    function openForTask(tId) {
        if (!tId || tId === "") return
        if (!projectController) return
        if (projectController.settingsManager.editingLocked) {
            if (mainWindow && mainWindow.toast) mainWindow.toast.show("Редактирование заблокировано")
            return
        }
        taskId = tId
        var task = projectController?.projectData?.taskModel?.getTask(tId)
        newNameField.text = task ? (task.title || "") : ""
        open()
    }

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 15

        Label { text: "Новое название задачи:"; Layout.fillWidth: true; font.pixelSize: 13 }
        TextField { id: newNameField; Layout.fillWidth: true; Layout.preferredHeight: 35
            placeholderText: "Введите название"; font.pixelSize: 13; focus: true }

        Item { Layout.fillHeight: true }

        RowLayout
        {
            Layout.fillWidth: true; spacing: 10
            Button { text: "Отмена"; Layout.fillWidth: true; Layout.preferredHeight: 35
                onClicked: root.close() }
            Button { text: "ОК"; Layout.fillWidth: true; Layout.preferredHeight: 35
                enabled: newNameField.text !== ""
                onClicked: {
                    if (root.taskId && projectController)
                        projectController.updateTaskTitle(root.taskId, newNameField.text)
                    root.close()
                } }
        }
    }
}
