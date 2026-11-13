import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0

Dialog
{
    id: root
    title: "Выберите зависимую задачу"
    width: 500; height: 400; modal: true
    standardButtons: Dialog.NoButton
    anchors.centerIn: Overlay.overlay

    property string sourceTaskId: ""
    property string targetTaskId: ""
    property bool downstream: true
    property var taskList: []

    function loadTasks() {
        taskList = []
        targetTaskId = ""
        if (!projectController || !projectController.projectData) { taskListView.model = []; return }
        var depModel = projectController.projectData.dependencyModel
        var isLocal = (projectController.projectData.graphKind === 1)

        if (isLocal) {
            var allTasks = projectController.projectData.taskModel.getAllTasks()
            for (var t = 0; t < allTasks.length; t++) {
                var tId = allTasks[t].id
                if (tId === sourceTaskId) continue
                var alreadyDepL = false
                for (var dd = 0; dd < depModel.rowCount(); dd++) {
                    var idxL = depModel.index(dd, 0)
                    var predL = depModel.data(idxL, Qt.UserRole + 2)
                    var succL = depModel.data(idxL, Qt.UserRole + 3)
                    if ((predL === sourceTaskId && succL === tId) ||
                        (predL === tId && succL === sourceTaskId)) { alreadyDepL = true; break }
                }
                if (!alreadyDepL)
                    taskList.push({ taskId: tId, title: allTasks[t].title, groupName: "" })
            }
        } else {
            var groupList = projectController.projectData.groupModel.getGroupIds()
            for (var g = 0; g < groupList.length; g++) {
                var groupId = groupList[g]
                var tasks = projectController.projectData.taskModel.getTasksForGroup(groupId)
                for (var i = 0; i < tasks.length; i++) {
                    if (tasks[i] === sourceTaskId) continue
                    var alreadyDep = false
                    for (var d = 0; d < depModel.rowCount(); d++) {
                        var idx = depModel.index(d, 0)
                        var pred = depModel.data(idx, Qt.UserRole + 2)
                        var succ = depModel.data(idx, Qt.UserRole + 3)
                        if ((pred === sourceTaskId && succ === tasks[i]) ||
                            (pred === tasks[i] && succ === sourceTaskId)) { alreadyDep = true; break }
                    }
                    if (!alreadyDep)
                        taskList.push({ taskId: tasks[i], title: projectController.projectData.taskModel.getTask(tasks[i]).title,
                                        groupName: projectController.projectData.groupModel.getGroup(groupId).name })
                }
            }
        }
        taskListView.model = taskList
    }

    function openForTask(taskId, isDownstream) {
        root.sourceTaskId = taskId
        root.downstream = isDownstream
        root.targetTaskId = ""
        root.open()
    }

    onOpened: loadTasks()

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 8

        ListView
        {
            id: taskListView
            Layout.fillWidth: true; Layout.fillHeight: true; clip: true
            delegate: Rectangle
            {
                width: parent.width; height: 40
                color: mouseArea.containsMouse ? "#e0e0e0" : "white"
                border.color: "#cccccc"; border.width: 1
                Text {
                    text: {
                        var suffix = (modelData.groupName && modelData.groupName !== "")
                                   ? " (" + modelData.groupName + ")" : ""
                        return modelData.title + suffix
                    }
                    anchors.left: parent.left; anchors.leftMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: 13
                }
                MouseArea {
                    id: mouseArea; anchors.fill: parent; hoverEnabled: true
                    onClicked: {
                        root.targetTaskId = modelData.taskId
                        if (root.downstream)
                            projectController.createDependency(root.sourceTaskId, root.targetTaskId)
                        else
                            projectController.createDependency(root.targetTaskId, root.sourceTaskId)
                        root.close()
                    }
                }
            }
        }

        RowLayout
        {
            Layout.alignment: Qt.AlignRight
            Button { text: "Отмена"; onClicked: root.close() }
        }
    }
}
