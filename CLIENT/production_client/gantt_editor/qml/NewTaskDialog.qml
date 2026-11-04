import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0

Dialog
{
    id: root
    title: "Создание новой задачи"
    width: 500
    height: 600
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    anchors.centerIn: Overlay.overlay

    property string groupId: ""
    property string taskTitle: "Новая задача"
    property string responsible: ""
    property date startDate: new Date()
    property date endDate: new Date()
    property string comment: ""
    property string insertAboveTaskId: ""
    property bool createWithoutDates: false

    function openForGroup(gId, insertAboveId)
    {
        groupId = gId
        insertAboveTaskId = insertAboveId || ""
        taskTitle = "Новая задача"
        responsible = ""
        startDate = new Date()
        endDate = new Date()
        endDate.setDate(endDate.getDate() + 7)
        comment = ""
        createWithoutDates = false
        withoutDatesCheck.checked = false
        titleField.text = taskTitle
        responsibleField.text = ""
        startDateField.text = Qt.formatDateTime(startDate, "dd.MM.yyyy")
        endDateField.text = Qt.formatDateTime(endDate, "dd.MM.yyyy")
        commentField.text = ""
        open()
    }

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 12

        Label { text: "Название задачи:"; font.bold: true }
        TextField { id: titleField; Layout.fillWidth: true; text: root.taskTitle; onTextChanged: root.taskTitle = text }

        Label { text: "Ответственный:"; font.bold: true }
        TextField { id: responsibleField; Layout.fillWidth: true; text: root.responsible; onTextChanged: root.responsible = text }

        CheckBox
        {
            id: withoutDatesCheck
            text: "Создать без сроков"
            onCheckedChanged: root.createWithoutDates = checked
        }

        ColumnLayout
        {
            Layout.fillWidth: true
            spacing: 12
            visible: !root.createWithoutDates

            Label { text: "Дата начала:"; font.bold: true }
            TextField
            {
                id: startDateField
                Layout.fillWidth: true
                onEditingFinished:
                {
                    var parts = text.split(".")
                    if (parts.length === 3)
                    {
                        var d = new Date(parseInt(parts[2]), parseInt(parts[1]) - 1, parseInt(parts[0]))
                        if (!isNaN(d.getTime())) root.startDate = d
                        else text = Qt.formatDateTime(root.startDate, "dd.MM.yyyy")
                    }
                    else text = Qt.formatDateTime(root.startDate, "dd.MM.yyyy")
                }
            }

            Label { text: "Дата завершения:"; font.bold: true }
            TextField
            {
                id: endDateField
                Layout.fillWidth: true
                onEditingFinished:
                {
                    var parts = text.split(".")
                    if (parts.length === 3)
                    {
                        var d = new Date(parseInt(parts[2]), parseInt(parts[1]) - 1, parseInt(parts[0]))
                        if (!isNaN(d.getTime())) root.endDate = d
                        else text = Qt.formatDateTime(root.endDate, "dd.MM.yyyy")
                    }
                    else text = Qt.formatDateTime(root.endDate, "dd.MM.yyyy")
                }
            }
        }

        Label { text: "Комментарий:"; font.bold: true }
        TextField
        {
            id: commentField
            Layout.fillWidth: true
            placeholderText: "Введите комментарий"
            text: root.comment
            onTextChanged: root.comment = text
        }

        Item { Layout.fillHeight: true }
    }

    onAccepted:
    {
        if (groupId && projectController)
        {
            if (root.createWithoutDates)
                projectController.taskController.addTaskWithoutDates(groupId, taskTitle, responsible)
            else
                projectController.taskController.addTask(groupId, taskTitle, responsible, startDate, endDate)

            if (comment)
            {
                var tasks = projectController.projectData.taskModel.getTasksForGroup(groupId)
                var newId = tasks[tasks.length - 1]
                projectController.projectData.taskModel.setTaskComment(newId, comment)
            }
            if (insertAboveTaskId)
            {
                var tasks2 = projectController.projectData.taskModel.getTasksForGroup(groupId)
                var newId2 = tasks2[tasks2.length - 1]
                var existingIndex = tasks2.indexOf(insertAboveTaskId)
                if (existingIndex >= 0)
                    projectController.projectData.taskModel.moveTaskToGroup(newId2, groupId, existingIndex)
            }
            if (mainWindow && mainWindow.gridArea) mainWindow.gridArea.updateData()
        }
    }
}
