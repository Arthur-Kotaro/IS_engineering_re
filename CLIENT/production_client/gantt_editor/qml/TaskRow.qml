import QtQuick 6.0
import QtQuick.Controls 6.0

Rectangle
{
    id: root
    height: 40
    color: (rowData && rowData.type === "task" && rowData.taskId === hoveredTaskId) ? "#4000aaff" : "transparent"

    property var rowData
    property string hoveredTaskId: ""
    property int rowHeight: 40
    property date displayStart: new Date()
    property int dayWidth: 30
    property bool showProgress: true
    property bool showComments: true
    property bool showTaskHistory: true
    property var timeLineRef: null
    property int updateCounter: 0
    property var externalFlickable: null
    property var taskContextMenu: null

    property bool tooltipVisible: false
    property var tooltipMouseArea: null
    property bool tooltipFromNode: false
    property var tooltipAnchorItem: null

    z: tooltipVisible ? 50 : 0

    // ---------- Заголовок группы ----------
    Text
    {
        visible: rowData && rowData.type === "group"
        text: rowData ? (rowData.name || "") : ""
        x:
        {
            var _ = root.updateCounter
            if (root.timeLineRef)
                return Math.max(10, root.timeLineRef.x + 10)
            return 10
        }
        anchors.verticalCenter: parent.verticalCenter
        font.bold: true
        font.pixelSize: 14
        z: 3
    }

    // ---------- Узел (TaskNode) ----------
    TaskNode
    {
        id: nodeCircle
        visible: rowData && rowData.type === "task"
                 && rowData.hasDates !== true
                 && rowData.isUnapproved !== true
        x: rowData ? ((rowData.nodeX || 0) - 15) : 0
        y: root.height / 2 - 15
        rowData: root.rowData
        taskContextMenu: root.taskContextMenu

        onTooltipRequested: function(visible, mouseArea, fromNode) {
            root.tooltipVisible = visible
            root.tooltipMouseArea = mouseArea
            root.tooltipFromNode = fromNode
            root.tooltipAnchorItem = nodeCircle
        }
        onTooltipHidden: {
            root.tooltipVisible = false
        }
    }

    // ---------- Полоса Ганта (TaskBar) ----------
    TaskBar
    {
        id: ganttBar
        visible: rowData && rowData.type === "task"
                 && (rowData.hasDates === true || rowData.isUnapproved === true)
        rowData: root.rowData
        displayStart: root.displayStart
        dayWidth: root.dayWidth
        showProgress: root.showProgress
        externalFlickable: root.externalFlickable
        taskContextMenu: root.taskContextMenu
        parentRow: root

        onTooltipRequested: function(visible, mouseArea, fromNode) {
            root.tooltipVisible = visible
            root.tooltipMouseArea = mouseArea
            root.tooltipFromNode = fromNode
            root.tooltipAnchorItem = ganttBar
        }
        onTooltipHidden: {
            root.tooltipVisible = false
        }
    }

    // ---------- Маркер конфликта ----------
    Rectangle
    {
        id: conflictMarker
        visible: rowData && rowData.type === "task" && rowData.hasConflict === true && rowData.hasDates === true
        x: ganttBar.x - 22
        y: ganttBar.y + (ganttBar.height - 16) / 2
        width: 16
        height: 16
        radius: 8
        color: "#FF4444"
        z: 6
        Text { anchors.centerIn: parent; text: "!"; color: "white"; font.pixelSize: 12; font.bold: true }
    }

    // ---------- Комментарий ----------
    Text
    {
        id: commentText
        visible: root.showComments && (rowData && rowData.type === "task")
        text: rowData ? (rowData.taskComment || "") : ""
        readonly property bool onNode: rowData && rowData.type === "task" && rowData.hasDates !== true && rowData.isUnapproved !== true
        x: onNode ? (nodeCircle.x + nodeCircle.width + 14) : (ganttBar.x + ganttBar.width + 14)
        y: onNode ? (nodeCircle.y + nodeCircle.height / 2 - height / 2) : (ganttBar.y + 4)
        font.pixelSize: 14
        color: "#666666"
        elide: Text.ElideRight
        width:
        {
            var baseX = onNode ? (nodeCircle.x + nodeCircle.width) : (ganttBar.x + ganttBar.width)
            return Math.max(40, Math.min(350, root.width - baseX - 14))
        }
    }

    // ---------- История переносов ----------
    Repeater
    {
        model:
        {
            if (rowData && rowData.type === "task" && rowData.rowKind === "target"
                && rowData.hasDates === true && root.showTaskHistory)
            {
                var task = projectController?.projectData?.taskModel?.getTask(rowData.taskId)
                return task && task.dateHistory ? task.dateHistory : []
            }
            return []
        }
        delegate: Rectangle
        {
            id: historyRect

            readonly property date oldStart: {
                if (!modelData || !modelData.old_start) return new Date()
                    return new Date(modelData.old_start)
            }
            readonly property date oldEnd: {
                if (!modelData || !modelData.old_end) return new Date()
                    return new Date(modelData.old_end)
            }

            x: Math.max(0, Math.floor((oldStart - root.displayStart) / 86400000) * root.dayWidth)
            width: Math.max(10, (Math.floor((oldEnd - oldStart) / 86400000) + 1) * root.dayWidth)
            height: ganttBar.height
            y: ganttBar.y
            color: "#cccccc"
            opacity: hoverArea.containsMouse ? 0.95 : 0.7
            radius: 4
            border.color: "#888888"
            border.width: 1
            z: -1

            MouseArea
            {
                id: hoverArea
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.NoButton
            }

            ToolTip
            {
                visible: hoverArea.containsMouse
                delay: 300
                text: {
                    if (!modelData) return ""
                        var s = (modelData.old_start || "") + " — " + (modelData.old_end || "")
                        var n = (modelData.new_start || "") + " — " + (modelData.new_end || "")
                        var r = modelData.reason || "—"
                        return "Было: " + s + "\nСтало: " + n + "\nПричина: " + r
                }
            }
        }
    }

    // ---------- Кастомный ToolTip ----------
    TaskTooltip
    {
        id: taskTooltip
        visible: root.tooltipVisible && rowData && rowData.type === "task"
        rowData: root.rowData
        mouseX: root.tooltipMouseArea ? root.tooltipMouseArea.mouseX : 0
        mouseY: root.tooltipMouseArea ? root.tooltipMouseArea.mouseY : 0
        fromNode: root.tooltipFromNode
        anchorItem: root.tooltipAnchorItem
    }
}
