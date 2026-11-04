import QtQuick 6.0
import QtQuick.Controls 6.0

Rectangle {
    id: root
    height: 30
    color: "#d0d0d0"

    property var stats: ({})
    property int conflicts: 0

    function refreshStats()
    {
        if (projectController && projectController.projectData)
            stats = projectController.projectData.getTaskStatistics()
        else
            stats = ({})
    }

    function refreshConflicts()
    {
        if (!projectController || !projectController.projectData)
        {
            conflicts = 0
            return
        }
        if (projectController.projectData.graphKind !== 1)
        {
            conflicts = 0
            return
        }
        conflicts = projectController.projectData.getConflictCount()
    }

    function getGroupsCount()
    {
        return (projectController && projectController.projectData && projectController.projectData.groupModel)
               ? projectController.projectData.groupModel.count : 0
    }

    function isModified()
    {
        return (projectController && projectController.projectData) ? projectController.projectData.modified : false
    }

    function conflictTooltip()
    {
        if (!projectController || !projectController.projectData) return ""
        var ids = projectController.projectData.getConflictingTaskIds()
        if (ids.length === 0) return ""
        var names = []
        for (var i = 0; i < ids.length; i++)
        {
            var t = projectController.projectData.taskModel.getTask(ids[i])
            if (t && t.title) names.push("• " + t.title)
        }
        return "Задачи, начинающиеся раньше завершения предшественника:\n" + names.join("\n")
    }

    Component.onCompleted:
    {
        refreshStats()
        refreshConflicts()
    }

    Connections
    {
        target: projectController
        function onProjectDataChanged() { refreshStats(); refreshConflicts() }
        function onProjectLoaded() { refreshStats(); refreshConflicts() }
    }

    Connections
    {
        target: projectController?.projectData?.taskModel ?? null
        function onCountChanged() { refreshStats(); refreshConflicts() }
        function onDataChanged(topLeft, bottomRight, roles) { refreshStats(); refreshConflicts() }
        function onRowsInserted(parent, first, last) { refreshStats(); refreshConflicts() }
        function onRowsRemoved(parent, first, last) { refreshStats(); refreshConflicts() }
        function onModelReset() { refreshStats(); refreshConflicts() }
    }

    Connections
    {
        target: projectController?.projectData ?? null
        function onLinkedPredecessorEndDateChanged() { refreshConflicts() }
        function onGraphKindChanged() { refreshConflicts() }
    }

    Row
    {
        anchors.fill: parent
        anchors.leftMargin: 10
        spacing: 20

        Text { text: "Групп: " + getGroupsCount(); anchors.verticalCenter: parent.verticalCenter }
        Text { text: "Всего задач: " + (stats.total !== undefined ? stats.total : 0); anchors.verticalCenter: parent.verticalCenter }
        Text { text: "Со сроками: " + (stats.withDates !== undefined ? stats.withDates : 0); anchors.verticalCenter: parent.verticalCenter }
        Text { text: "Без сроков: " + (stats.withoutDates !== undefined ? stats.withoutDates : 0); anchors.verticalCenter: parent.verticalCenter }
        Text { text: "Неутверждённых: " + (stats.unapprovedForecasts !== undefined ? stats.unapprovedForecasts : 0); anchors.verticalCenter: parent.verticalCenter }

        Rectangle
        {
            id: conflictBadge
            visible: conflicts > 0
            width: conflictText.implicitWidth + 20
            height: 22
            radius: 4
            color: "#FF4444"
            anchors.verticalCenter: parent.verticalCenter

            Text
            {
                id: conflictText
                anchors.centerIn: parent
                text: "⚠ Конфликтов с предшественником: " + root.conflicts
                color: "white"
                font.pixelSize: 12
                font.bold: true
            }

            MouseArea
            {
                id: conflictMouse
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.NoButton
            }

            ToolTip
            {
                visible: conflictBadge.visible && conflictMouse.hovered
                delay: 300
                text: root.conflictTooltip()
            }
        }

        Rectangle { width: 10; height: 10; radius: 5; color: isModified() ? "orange" : "green"; anchors.verticalCenter: parent.verticalCenter }
        Text { text: isModified() ? "Изменён" : "Сохранён"; anchors.verticalCenter: parent.verticalCenter }
    }
}
