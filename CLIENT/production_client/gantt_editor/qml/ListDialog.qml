import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0

Dialog
{
    id: root
    title: "Создание списка задач"
    width: 1380
    height: 600
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    anchors.centerIn: Overlay.overlay

    property int taskCount: 0
    property var listModel: []
    property bool createWithoutDates: false
    property bool isLocalMode: projectController && projectController.projectData && projectController.projectData.graphKind === 1

    function groupNames()
    {
        if (!projectController || !projectController.projectData) return []
        return projectController.projectData.groupModel.getGroupIds().map(function(id)
        {
            return projectController.projectData.groupModel.getGroup(id).name
        })
    }

    function buildList()
    {
        listModel = []
        for (var i = 0; i < taskCount; i++)
        {
            listModel.push({
                groupIndex: 0,
                title: "Задача " + (i + 1),
                responsible: "",
                startDate: new Date(),
                duration: 7,
                comment: ""
            })
        }
        listRepeater.model = listModel
    }

    function resetDialog()
    {
        taskCount = 0
        listModel = []
        countField.text = "3"
        createWithoutDates = false
        withoutDatesCheck.checked = false
        listRepeater.model = []
    }

    onOpened:
    {
        resetDialog()
        countField.forceActiveFocus()
    }

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 10

        RowLayout
        {
            Layout.fillWidth: true
            spacing: 10

            Label { text: "Количество задач:" }
            TextField
            {
                id: countField
                Layout.preferredWidth: 60
                text: "3"
                validator: IntValidator { bottom: 1; top: 100 }
                onEditingFinished:
                {
                    root.taskCount = parseInt(text) || 3
                    root.buildList()
                }
            }

            CheckBox
            {
                id: withoutDatesCheck
                text: "Создать без сроков"
                onCheckedChanged: root.createWithoutDates = checked
            }

            Button
            {
                text: "Построить"
                onClicked:
                {
                    root.taskCount = parseInt(countField.text) || 3
                    root.buildList()
                }
            }

            Item { Layout.fillWidth: true }
        }

        Rectangle { Layout.fillWidth: true; height: 1; color: "#cccccc" }

        ScrollView
        {
            id: scrollView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            ColumnLayout
            {
                width: scrollView.availableWidth
                spacing: 5

                Repeater
                {
                    id: listRepeater
                    model: listModel

                    delegate: RowLayout
                    {
                        Layout.fillWidth: true
                        width: parent.width
                        spacing: 5

                        Label { text: (index + 1) + "."; Layout.preferredWidth: 25 }

                        ComboBox
                        {
                            Layout.preferredWidth: 300
                            visible: !root.isLocalMode
                            model: root.groupNames()
                            onCurrentIndexChanged:
                            {
                                if (listModel[index]) listModel[index].groupIndex = currentIndex
                            }
                        }

                        TextField
                        {
                            Layout.preferredWidth: 200
                            text: modelData ? modelData.title : ""
                            placeholderText: "Название"
                            onTextChanged:
                            {
                                if (listModel[index]) listModel[index].title = text
                            }
                        }

                        TextField
                        {
                            Layout.fillWidth: true
                            Layout.minimumWidth: 150
                            placeholderText: "Ответственный"
                            text: modelData ? modelData.responsible : ""
                            onTextChanged:
                            {
                                if (listModel[index]) listModel[index].responsible = text
                            }
                        }

                        TextField
                        {
                            Layout.preferredWidth: 100
                            visible: !root.createWithoutDates
                            placeholderText: "ДД.ММ.ГГГГ"
                            text: modelData ? Qt.formatDateTime(modelData.startDate, "dd.MM.yyyy") : ""
                            onEditingFinished:
                            {
                                var parts = text.split(".")
                                if (parts.length === 3)
                                {
                                    var d = new Date(parseInt(parts[2]), parseInt(parts[1]) - 1, parseInt(parts[0]))
                                    if (!isNaN(d.getTime()) && listModel[index])
                                        listModel[index].startDate = d
                                }
                            }
                        }

                        TextField
                        {
                            Layout.preferredWidth: 50
                            visible: !root.createWithoutDates
                            text: modelData ? modelData.duration : "7"
                            validator: IntValidator { bottom: 1; top: 365 }
                            onTextChanged:
                            {
                                if (listModel[index]) listModel[index].duration = parseInt(text) || 7
                            }
                        }

                        Label
                        {
                            text: "дн."
                            visible: !root.createWithoutDates
                            Layout.preferredWidth: 30
                        }

                        TextField
                        {
                            Layout.fillWidth: true
                            Layout.minimumWidth: 200
                            placeholderText: "Комментарий"
                            text: modelData ? modelData.comment : ""
                            onTextChanged:
                            {
                                if (listModel[index]) listModel[index].comment = text
                            }
                        }
                    }
                }
            }
        }
    }

    onAccepted:
    {
        if (listModel.length === 0) return

        var groupIds = projectController.projectData.groupModel.getGroupIds()

        for (var i = 0; i < listModel.length; i++)
        {
            var item = listModel[i]
            var groupId = groupIds[item.groupIndex] || groupIds[0]
            var newTaskId = ""

            if (root.createWithoutDates)
            {
                projectController.taskController.addTaskWithoutDates(groupId, item.title, item.responsible)
                var tasks = projectController.projectData.taskModel.getTasksForGroup(groupId)
                newTaskId = tasks[tasks.length - 1]
            }
            else
            {
                var endDate = new Date(item.startDate)
                endDate.setDate(endDate.getDate() + item.duration - 1)
                projectController.taskController.addTask(groupId, item.title, item.responsible, item.startDate, endDate)
                var tasks2 = projectController.projectData.taskModel.getTasksForGroup(groupId)
                newTaskId = tasks2[tasks2.length - 1]
            }

            if (item.comment)
                projectController.projectData.taskModel.setTaskComment(newTaskId, item.comment)
        }

        if (mainWindow && mainWindow.gridArea) mainWindow.gridArea.updateData()
    }
}
