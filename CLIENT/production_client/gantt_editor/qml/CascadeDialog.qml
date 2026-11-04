import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0

Dialog
{
    id: root
    title: "Создание каскада задач"
    width: 1380
    height: 600
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    anchors.centerIn: Overlay.overlay

    property int taskCount: 0
    property date startDate: new Date()
    property var cascadeModel: []
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

    function buildCascade()
    {
        cascadeModel = []
        for (var i = 0; i < taskCount; i++)
        {
            cascadeModel.push({
                groupIndex: 0,
                title: "Задача " + (i + 1),
                responsible: "",
                duration: 7,
                comment: ""
            })
        }
        cascadeRepeater.model = cascadeModel
    }

    function resetDialog()
    {
        taskCount = 0
        cascadeModel = []
        countField.text = "3"
        startDate = new Date()
        startDateField.text = Qt.formatDateTime(root.startDate, "dd.MM.yyyy")
        createWithoutDates = false
        withoutDatesCheck.checked = false
        cascadeRepeater.model = []
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
                    root.buildCascade()
                }
            }

            Label
            {
                text: "Дата начала:"
                visible: !root.createWithoutDates
            }
            TextField
            {
                id: startDateField
                Layout.preferredWidth: 100
                visible: !root.createWithoutDates
                onEditingFinished:
                {
                    var parts = text.split(".")
                    if (parts.length === 3)
                    {
                        var d = new Date(parseInt(parts[2]), parseInt(parts[1]) - 1, parseInt(parts[0]))
                        if (!isNaN(d.getTime())) root.startDate = d
                    }
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
                    root.buildCascade()
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
                    id: cascadeRepeater
                    model: cascadeModel

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
                                if (cascadeModel[index]) cascadeModel[index].groupIndex = currentIndex
                            }
                        }

                        TextField
                        {
                            Layout.preferredWidth: 200
                            text: modelData ? modelData.title : ""
                            placeholderText: "Название"
                            onTextChanged:
                            {
                                if (cascadeModel[index]) cascadeModel[index].title = text
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
                                if (cascadeModel[index]) cascadeModel[index].responsible = text
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
                                if (cascadeModel[index]) cascadeModel[index].duration = parseInt(text) || 7
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
                                if (cascadeModel[index]) cascadeModel[index].comment = text
                            }
                        }
                    }
                }
            }
        }
    }

    onAccepted:
    {
        if (cascadeModel.length === 0) return

        var currentDate = new Date(root.startDate)
        var prevTaskId = ""
        var groupIds = projectController.projectData.groupModel.getGroupIds()

        for (var i = 0; i < cascadeModel.length; i++)
        {
            var item = cascadeModel[i]
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
                var endDate = new Date(currentDate)
                endDate.setDate(endDate.getDate() + item.duration - 1)
                projectController.taskController.addTask(groupId, item.title, item.responsible, currentDate, endDate)
                var tasks2 = projectController.projectData.taskModel.getTasksForGroup(groupId)
                newTaskId = tasks2[tasks2.length - 1]
                currentDate = new Date(endDate)
                currentDate.setDate(currentDate.getDate() + 1)
            }

            if (item.comment)
                projectController.projectData.taskModel.setTaskComment(newTaskId, item.comment)

            if (prevTaskId)
                projectController.dependencyController.addDependency(prevTaskId, newTaskId)

            prevTaskId = newTaskId
        }

        if (mainWindow && mainWindow.gridArea) mainWindow.gridArea.updateData()
    }
}
