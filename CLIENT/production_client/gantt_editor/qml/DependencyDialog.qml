import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0

Dialog
{
    id: root
    title: "Выберите зависимую задачу"
    width: 500
    height: 400
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    anchors.centerIn: Overlay.overlay

    property string sourceTaskId: ""
    property string targetTaskId: ""
    property bool downstream: true

    property var taskList: []
    property var groupList: []

    function loadTasks()
    {
        taskList = []
        targetTaskId = ""

        if (!projectController || !projectController.projectData) {
            taskListView.model = []
            return
        }

        var depModel = projectController.projectData.dependencyModel
        var isLocal = (projectController.projectData.graphKind === 1)

        if (isLocal)
        {
            // Локальный график: групп нет, берём все задачи напрямую
            var allTasks = projectController.projectData.taskModel.getAllTasks()
            for (var t = 0; t < allTasks.length; t++)
            {
                var tId = allTasks[t].id
                if (tId === sourceTaskId) continue

                var alreadyDepL = false
                for (var dd = 0; dd < depModel.rowCount(); dd++)
                {
                    var idxL = depModel.index(dd, 0)
                    var predL = depModel.data(idxL, Qt.UserRole + 2)
                    var succL = depModel.data(idxL, Qt.UserRole + 3)
                    if ((predL === sourceTaskId && succL === tId) ||
                        (predL === tId && succL === sourceTaskId))
                    {
                        alreadyDepL = true
                        break
                    }
                }

                if (!alreadyDepL)
                {
                    taskList.push({
                        taskId: tId,
                        groupId: "",
                        title: allTasks[t].title,
                        groupName: ""
                    })
                }
            }
        }
        else
        {
            // Мастерграфик: перебираем группы
            groupList = projectController.projectData.groupModel.getGroupIds()
            for (var g = 0; g < groupList.length; g++)
            {
                var groupId = groupList[g]
                var tasks = projectController.projectData.taskModel.getTasksForGroup(groupId)
                for (var i = 0; i < tasks.length; i++)
                {
                    if (tasks[i] === sourceTaskId) continue

                    var alreadyDep = false
                    for (var d = 0; d < depModel.rowCount(); d++)
                    {
                        var idx = depModel.index(d, 0)
                        var pred = depModel.data(idx, Qt.UserRole + 2)
                        var succ = depModel.data(idx, Qt.UserRole + 3)
                        if ((pred === sourceTaskId && succ === tasks[i]) ||
                            (pred === tasks[i] && succ === sourceTaskId))
                        {
                            alreadyDep = true
                            break
                        }
                    }

                    if (!alreadyDep)
                    {
                        taskList.push({
                            taskId: tasks[i],
                            groupId: groupId,
                            title: projectController.projectData.taskModel.getTask(tasks[i]).title,
                            groupName: projectController.projectData.groupModel.getGroup(groupId).name
                        })
                    }
                }
            }
        }

        taskListView.model = taskList
    }

    function openForTask(taskId, isDownstream)
    {
        root.sourceTaskId = taskId
        root.downstream = isDownstream
        root.targetTaskId = ""
        root.open()
    }

    onOpened: loadTasks()

    onAccepted:
    {
        if (targetTaskId && sourceTaskId)
        {
            if (downstream)
                projectController.dependencyController.addDependency(sourceTaskId, targetTaskId)
            else
                projectController.dependencyController.addDependency(targetTaskId, sourceTaskId)
        }
    }

    ListView
    {
        id: taskListView
        anchors.fill: parent
        anchors.margins: 10
        clip: true
        delegate: Rectangle
        {
            width: parent.width
            height: 40
            color: mouseArea.containsMouse ? "#e0e0e0" : "white"
            border.color: "#cccccc"
            border.width: 1

            Text
            {
                text:
                {
                    var suffix = (modelData.groupName && modelData.groupName !== "")
                               ? " (" + modelData.groupName + ")"
                               : ""
                    return modelData.title + suffix
                }
                anchors.left: parent.left
                anchors.leftMargin: 10
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: 13
            }

            MouseArea
            {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
                onClicked:
                {
                    root.targetTaskId = modelData.taskId
                    root.accept()
                }
            }
        }
    }
}
