import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0
import "utils/RowBuilder.js" as RowBuilder

Item
{
    id: root
    height: groupHeader.height + (expanded ? tasksColumn.height : 0)
    property string groupId: model.groupId
    property string groupName: model.groupName
    property bool expanded: model.expanded
    property var flickableRight: null

    property var rowsModel: []

    function currentViewMode()
    {
        return projectController && projectController.settingsManager
               ? projectController.settingsManager.viewMode : 0
    }

    function refreshTasks()
    {
        if (!projectController || !projectController.projectData || !groupId)
        {
            rowsModel = []
            return
        }
        var mode = currentViewMode()
        var taskIds = projectController.projectData.taskModel.getTasksForGroup(groupId)
        rowsModel = RowBuilder.buildGroupRows(projectController.projectData.taskModel, taskIds, mode)
    }

    Component.onCompleted: refreshTasks()

    Connections
    {
        target: projectController && projectController.projectData ? projectController.projectData.taskModel : null
        function onCountChanged() { refreshTasks() }
        function onRowsInserted() { refreshTasks() }
        function onRowsRemoved() { refreshTasks() }
        function onDataChanged() { refreshTasks() }
    }

    Connections
    {
        target: projectController && projectController.projectData ? projectController.projectData.groupModel : null
        function onDataChanged() { refreshTasks() }
        function onGroupExpandedChanged(changedGroupId)
        {
            if (changedGroupId === groupId) refreshTasks()
        }
        function onRowsInserted() { refreshTasks() }
        function onRowsRemoved() { refreshTasks() }
    }

    Connections
    {
        target: projectController?.settingsManager
        function onViewModeChanged() { refreshTasks() }
    }

    Rectangle
    {
        id: groupHeader
        width: parent.width
        height: 40
        color: "#e0e0e0"
        border.color: "#cccccc"
        border.width: 1

        Row
        {
            anchors.fill: parent
            anchors.leftMargin: 5
            spacing: 5

            Rectangle
            {
                width: 24; height: 24; color: "transparent"; anchors.verticalCenter: parent.verticalCenter
                Text { text: expanded ? "▼" : "▶"; anchors.centerIn: parent; font.pixelSize: 12 }
                MouseArea
                {
                    anchors.fill: parent
                    onClicked: if (projectController && projectController.projectData) projectController.projectData.groupModel.setGroupExpanded(groupId, !root.expanded)
                }
            }

            Text
            {
                text: groupName
                font.pixelSize: 14
                font.bold: true
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width - 30
                elide: Text.ElideRight
            }
        }

        MouseArea
        {
            anchors.fill: parent
            acceptedButtons: Qt.RightButton
            onClicked: function(mouse)
            {
                if (mouse.button === Qt.RightButton) {
                    groupContextMenu.groupId = groupId
                    groupContextMenu.groupName = groupName
                    groupContextMenu.popup()
                }
            }
        }

        GroupContextMenu
        {
            id: groupContextMenu
            onAddTaskClicked: {
                if (projectController && projectController.projectData) {
                    mainWindow.newTaskDialog.openForGroup(groupId)
                    refreshTasks()
                }
            }
            onRenameClicked: renameGroupDialog.openWithGroup(groupId, groupName)
            onAddAboveClicked: addGroupAbove()
            onAddBelowClicked: addGroupBelow()
            onDeleteClicked: deleteGroup()
        }
    }

    Column
    {
        id: tasksColumn
        y: groupHeader.height
        width: parent.width
        visible: root.expanded
        height: rowsRepeater.count * 40

        Repeater
        {
            id: rowsRepeater
            model: root.rowsModel

            delegate: Rectangle
            {
                width: root.width
                height: 40
                color: "white"
                border.color: "#eeeeee"
                border.width: 1

                property string taskId: modelData ? modelData.taskId : ""
                property string rowKind: modelData ? modelData.rowKind : "target"
                property var taskData: modelData ? modelData.data : null

                function isValidDate(d) { return d !== undefined && d !== null && !isNaN(new Date(d).getTime()) }
                function hasDates() { return taskData && isValidDate(taskData.startDate) && isValidDate(taskData.endDate) }

                function titleText() {
                    if (!taskData) return ""
                    var prefix = ""
                    if (taskData.localGraphState !== undefined && taskData.localGraphState !== 0) prefix = "🔗 "
                    return prefix + taskData.title
                }

                function responsibleText() {
                    if (!taskData) return ""
                    var r = taskData.responsible
                    return (r && r.length > 0) ? r : "—"
                }

                function startDateText() {
                    if (!taskData) return "—"
                    var d = (rowKind === "forecast" && !modelData.isCompleted) ? taskData.forecastStart : taskData.startDate
                    if (!isValidDate(d)) return "—"
                    return Qt.formatDateTime(new Date(d), "dd.MM.yyyy")
                }

                function endDateText() {
                    if (!taskData) return "—"
                    var d = (rowKind === "forecast" && !modelData.isCompleted) ? taskData.forecastEnd : taskData.endDate
                    if (!isValidDate(d)) return "—"
                    return Qt.formatDateTime(new Date(d), "dd.MM.yyyy")
                }

                function viewLabelText() {
                    if (rowKind === "forecast") return "Прогноз"
                    if (modelData && modelData.isCompleted) return "Факт"
                    if (!hasDates()) return "Драфт"
                    return "Цель"
                }

                Row
                {
                    anchors.fill: parent
                    anchors.leftMargin: 5
                    anchors.rightMargin: 5
                    spacing: 0

                    Rectangle
                    {
                        width: parent.width - 265
                        height: parent.height
                        color: "transparent"
                        Row
                        {
                            anchors.fill: parent
                            Rectangle
                            {
                                width: parent.width * 0.64; height: parent.height; color: "transparent"
                                Text {
                                    text: titleText(); anchors.left: parent.left; anchors.leftMargin: 10
                                    anchors.verticalCenter: parent.verticalCenter; elide: Text.ElideRight
                                    width: parent.width - 20; font.pixelSize: 12; color: "#222222"
                                }
                            }
                            Rectangle
                            {
                                width: parent.width * 0.36; height: parent.height; color: "transparent"
                                Text {
                                    text: responsibleText(); anchors.left: parent.left; anchors.leftMargin: 20
                                    anchors.verticalCenter: parent.verticalCenter; elide: Text.ElideRight
                                    width: parent.width - 10; font.pixelSize: 12
                                }
                            }
                        }
                    }

                    Rectangle {
                        width: 65; height: parent.height; color: "transparent"
                        Text { text: viewLabelText(); anchors.left: parent.left; anchors.leftMargin: 4; anchors.verticalCenter: parent.verticalCenter; font.pixelSize: 12; color: "#444444" }
                    }
                    Rectangle {
                        width: 100; height: parent.height; color: "transparent"
                        Text { text: startDateText(); anchors.left: parent.left; anchors.leftMargin: 30; anchors.verticalCenter: parent.verticalCenter; font.pixelSize: 12 }
                    }
                    Rectangle {
                        width: 100; height: parent.height; color: "transparent"
                        Text { text: endDateText(); anchors.left: parent.left; anchors.leftMargin: 30; anchors.verticalCenter: parent.verticalCenter; font.pixelSize: 12 }
                    }
                }

                MouseArea
                {
                    id: rowMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    cursorShape: Qt.ArrowCursor
                    
                    onPositionChanged: {
                        if (mainWindow && mainWindow.hasGridArea()) {
                            if (mainWindow.gridArea.hoveredTaskId !== taskId)
                                mainWindow.gridArea.hoveredTaskId = taskId
                        }
                    }
                    
                    onExited: {
                        if (mainWindow && mainWindow.hasGridArea() && mainWindow.gridArea.hoveredTaskId === taskId) {
                            mainWindow.gridArea.hoveredTaskId = ""
                        }
                    }
                    
                    onDoubleClicked: {
                        if (mainWindow && mainWindow.hasGridArea()) 
                            mainWindow.gridArea.centerOnTask(taskId)
                    }
                    
                    onClicked: function(mouse)
                    {
                        if (mouse.button === Qt.RightButton && taskId)
                        {
                            taskContextMenu.taskId = taskId
                            taskContextMenu.popup()
                        }
                    }
                }

                TaskContextMenu
                {
                    id: taskContextMenu
                    onAddTaskAboveCallback: function(tId) { addTaskAbove(tId) }
                    onAddTaskBelowCallback: function(tId) { addTaskBelow(tId) }
                }
            }
        }
    }

    RenameGroupDialog { id: renameGroupDialog }

    function addTaskAbove(existingTaskId) { mainWindow.newTaskDialog.openForGroup(groupId, existingTaskId) }
    
    function addTaskBelow(existingTaskId)
    {
        var tasks = projectController.projectData.taskModel.getTasksForGroup(groupId)
        var index = tasks.indexOf(existingTaskId)
        var insertAfter = (index >= 0 && index + 1 < tasks.length) ? tasks[index + 1] : ""
        mainWindow.newTaskDialog.openForGroup(groupId, insertAfter)
    }

    function addGroupAbove()
    {
        if (projectController && projectController.projectData) {
            var groupIds = projectController.projectData.groupModel.getGroupIds()
            var index = groupIds.indexOf(groupId)
            if (index < 0) index = 0
            projectController.projectData.groupModel.addGroupAt(index, "Новая группа")
        }
    }

    function addGroupBelow()
    {
        if (projectController && projectController.projectData) {
            var groupIds = projectController.projectData.groupModel.getGroupIds()
            var index = groupIds.indexOf(groupId)
            if (index < 0) index = groupIds.length
            projectController.projectData.groupModel.addGroupAt(index + 1, "Новая группа")
        }
    }

    function deleteGroup()
    {
        if (projectController && projectController.projectData) {
            var tasks = projectController.projectData.taskModel.getTasksForGroup(groupId)
            if (tasks.length > 0) {
                // Показать toast
                if (mainWindow && mainWindow.toast) mainWindow.toast.show("Удаление групп содержащих задачи запрещено")
                    return
            }
            projectController.projectData.groupModel.removeGroup(groupId)
        }
    }
}
