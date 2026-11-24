import QtQuick 6.0
import QtQuick.Controls 6.0
import "utils/GridRange.js" as GridRange
import "utils/GridLayoutBuilder.js" as GridLayoutBuilder

Rectangle
{
    id: root

    color: "white"

    property int rowHeight: 40
    property var externalFlickable: null
    property bool showDependencies: true
    property bool showComments: true
    property bool showTaskHistory: true
    property bool showProgress: true
    property date displayStart: new Date()
    property date displayEnd: new Date()
    property int dayWidth: projectController && projectController.settingsManager.zoomLevel === 1 ? 10 : 30
    property int totalDays: 1
    property real gridWidth: totalDays * dayWidth
    property int totalRows: 1
    property real contentHeight: 0
    property var visibleItems: []
    property int updateCounter: 0
    property var taskContextMenu: null
    property string hoveredTaskId: ""
    property var timeLineRef: currentTimeLine

    property bool showTargetStartLine: false
    property bool showTargetEndLine: false
    property real targetStartLineX: 0
    property real targetEndLineX: 0
    property bool targetStartLineArrows: true
    property bool targetEndLineArrows: true

    property bool showPredecessorLine: false
    property real predecessorLineX: 0

    property int effectiveRows: totalRows

    readonly property bool isPure: projectController ? projectController.isPure : false
    readonly property bool canEdit: projectController ? projectController.canEdit : false

    onShowDependenciesChanged: dependencyCanvas.refresh()
    onDayWidthChanged:
    {
        updateData()
        backgroundCanvas.refresh()
        dependencyCanvas.refresh()
    }

    readonly property bool isLocalMode:
    {
        return projectController && projectController.projectData && projectController.projectData.graphKind === 1
    }

    Component.onCompleted: updateData()

    CurrentTimeLine
    {
        id: currentTimeLine
        displayStart: root.displayStart
        dayWidth: root.dayWidth
    }

    TargetDatesLine
    {
        id: targetStartLine
        visible: root.isLocalMode && root.showTargetStartLine
        x: root.targetStartLineX
        showArrows: root.targetStartLineArrows
        arrowDirection: "right"
        lineColor: "#2266cc"
    }

    TargetDatesLine
    {
        id: targetEndLine
        visible: root.isLocalMode && root.showTargetEndLine
        x: root.targetEndLineX
        showArrows: root.targetEndLineArrows
        arrowDirection: "left"
        lineColor: "#2266cc"
    }

    TargetDatesLine
    {
        id: predecessorLine
        visible: root.isLocalMode && root.showPredecessorLine
        x: root.predecessorLineX
        showArrows: true
        arrowDirection: "right"
        lineColor: "#FF4444"
    }

    Connections
    {
        target: projectController
        function onProjectDataChanged() { updateData() }
        function onProjectLoaded() { updateData() }
        function onIsPureChanged() { updateData() }
    }

    Connections
    {
        target: projectController?.settingsManager ?? null
        function onViewModeChanged() { updateData() }
    }

    Connections
    {
        target: projectController?.projectData ?? null
        function onGraphKindChanged() { updateData() }
        function onLinkedPredecessorEndDateChanged() { updateData() }
    }

    function currentViewMode()
    {
        if (isLocalMode) return 0
        return projectController && projectController.settingsManager ? projectController.settingsManager.viewMode : 0
    }

    function updateTargetDateLines()
    {
        showTargetStartLine = false
        showTargetEndLine = false
        showPredecessorLine = false
        targetStartLineX = 0
        targetEndLineX = 0
        predecessorLineX = 0
        targetStartLineArrows = true
        targetEndLineArrows = true

        if (!isLocalMode) return
        if (!projectController || !projectController.projectData) return

        var linkedStart = projectController.projectData.getLinkedTargetStart()
        var linkedEnd = projectController.projectData.getLinkedTargetEnd()
        var predEnd = projectController.projectData.getLinkedPredecessorEndDate()

        if (GridRange.isValidDate(linkedStart))
        {
            var xs = GridRange.dateToX(linkedStart, root.displayStart, root.dayWidth)
            if (xs >= 0) { targetStartLineX = xs; showTargetStartLine = true }
        }

        if (GridRange.isValidDate(linkedEnd))
        {
            var xe = GridRange.dateToX(linkedEnd, root.displayStart, root.dayWidth)
            if (xe >= 0) { targetEndLineX = xe + root.dayWidth; showTargetEndLine = true }
        }

        if (GridRange.isValidDate(predEnd))
        {
            var xp = GridRange.dateToX(predEnd, root.displayStart, root.dayWidth)
            if (xp >= 0) { predecessorLineX = xp + root.dayWidth; showPredecessorLine = true }
        }
    }

    function updateData()
    {
        if (!projectController || !projectController.projectData) return
        console.log("[GridArea] updateData START")
        if (isPure)
        {
            var now = new Date()
            var s = new Date(now.getFullYear(), now.getMonth(), 1)
            var e = new Date(now.getFullYear(), now.getMonth() + 2, 0)
            while (s.getDay() !== 1) s.setDate(s.getDate() - 1)
            s.setHours(0, 0, 0, 0)
            e.setHours(23, 59, 59, 999)

            displayStart = s
            displayEnd = e
            totalDays = Math.max(1, Math.floor((e - s) / 86400000) + 1)
            gridWidth = totalDays * dayWidth

            visibleItems = []
            updateCounter++
            totalRows = 5
            effectiveRows = 5
            contentHeight = 5 * rowHeight
            root.height = contentHeight
            tasksRepeater.model = []

            updateTargetDateLines()
            backgroundCanvas.refresh()
            dependencyCanvas.refresh()
            if (currentTimeLine) currentTimeLine.updateLinePosition()
            return
        }

        var localMode = (projectController.projectData.graphKind === 1)

        if (localMode)
        {
            var linkedStart = projectController.projectData.getLinkedTargetStart()
            var linkedEnd = projectController.projectData.getLinkedTargetEnd()
            var earliest = projectController.projectData.getEarliestDate()
            var latest = projectController.projectData.getLatestDate()
            var predEnd = projectController.projectData.getLinkedPredecessorEndDate()

            var dates = []

            if (GridRange.isValidDate(linkedStart)) dates.push(new Date(linkedStart))
            if (GridRange.isValidDate(linkedEnd)) dates.push(new Date(linkedEnd))
            if (GridRange.isValidDate(earliest)) dates.push(new Date(earliest))
            if (GridRange.isValidDate(latest)) dates.push(new Date(latest))
            if (GridRange.isValidDate(predEnd)) dates.push(new Date(predEnd))

            var s, e

            if (dates.length === 0)
            {
                var now = new Date()
                s = new Date(now.getFullYear(), now.getMonth(), 1)
                e = new Date(now.getFullYear(), now.getMonth() + 2, 0)
            }
            else
            {
                var minD = dates[0]
                var maxD = dates[0]
                for (var i = 1; i < dates.length; i++) {
                    if (dates[i] < minD) minD = dates[i]
                    if (dates[i] > maxD) maxD = dates[i]
                }
                s = new Date(minD)
                e = new Date(maxD)
            }

            s.setDate(s.getDate() - 14)
            s.setHours(0, 0, 0, 0)
            e.setDate(e.getDate() + 14)
            e.setHours(23, 59, 59, 999)

            while (s.getDay() !== 1) s.setDate(s.getDate() - 1)

            displayStart = s
            displayEnd = e
            totalDays = Math.max(1, Math.floor((e - s) / 86400000) + 1)
            gridWidth = totalDays * dayWidth
        }
        else
        {
            var earliest = projectController.projectData.getEarliestDate()
            var latest = projectController.projectData.getLatestDate()

            if (earliest && latest)
            {
                var start = new Date(earliest)
                while (start.getDay() !== 1) start.setDate(start.getDate() - 1)
                start.setDate(start.getDate() - 28); start.setHours(0, 0, 0, 0)
                displayStart = start

                var end = new Date(latest)
                while (end.getDay() !== 0) end.setDate(end.getDate() + 1)
                end.setDate(end.getDate() + 14); end.setHours(23, 59, 59, 999)
                displayEnd = end

                totalDays = Math.max(1, Math.floor((end - start) / 86400000) + 1)
                gridWidth = totalDays * dayWidth
            }
        }

        var conflictSet = {}
        if (localMode)
        {
            var conflictIds = projectController.projectData.getConflictingTaskIds()
            for (var ci = 0; ci < conflictIds.length; ci++) conflictSet[conflictIds[ci]] = true
        }

        visibleItems = GridLayoutBuilder.buildLayout({
            localMode: localMode,
            displayStart: displayStart,
            dayWidth: dayWidth,
            mode: currentViewMode(),
            conflictSet: conflictSet,
            projectData: projectController.projectData
        })

        updateCounter++
        totalRows = visibleItems.length

        var rowsForHeight = (localMode) ? Math.max(totalRows, 5) : totalRows
        effectiveRows = rowsForHeight
        contentHeight = rowsForHeight * rowHeight

        root.height = contentHeight

        tasksRepeater.model = visibleItems
        console.log("[GridArea] isPure=", isPure,
                    " localMode=", localMode,
                    " displayStart=", displayStart,
                    " displayEnd=", displayEnd,
                    " totalDays=", totalDays,
                    " gridWidth=", gridWidth,
                    " totalRows=", totalRows,
                    " contentHeight=", contentHeight,
                    " visibleItems=", visibleItems.length,
                    " width=", root.width)
        updateTargetDateLines()
        backgroundCanvas.refresh()
        dependencyCanvas.refresh()
        if (currentTimeLine) currentTimeLine.updateLinePosition()
    }

    function centerOnTask(taskId)
    {
        if (!externalFlickable) return

        var targetCenterX = -1
        var targetRow = -1

        for (var i = 0; i < visibleItems.length; i++) {
            if (visibleItems[i].type === "task" && visibleItems[i].taskId === taskId) {
                var item = visibleItems[i]
                var isTargetRow = (item.rowKind === "target")
                var isForecastRow = (item.rowKind === "forecast")
                var currentMode = currentViewMode()

                if (currentMode !== 0 && !isTargetRow && !isForecastRow) continue

                if (currentMode !== 0 && isTargetRow || currentMode === 0) {
                    if (item.hasDates === true || item.isUnapproved === true) {
                        var startVal = (item.hasDates === true) ? item.taskStart : item.forecastStart
                        var endVal = (item.hasDates === true) ? item.taskEnd : item.forecastEnd

                        if (GridRange.isValidDate(startVal) && GridRange.isValidDate(endVal)) {
                            var xStart = GridRange.dateToX(startVal, displayStart, dayWidth)
                            var xEnd = GridRange.dateToX(endVal, displayStart, dayWidth) + dayWidth
                            targetCenterX = xStart + (xEnd - xStart) / 2
                            targetRow = i
                            break
                        }
                    } else if (item.hasDates !== true && item.isUnapproved !== true) {
                        targetCenterX = item.nodeX
                        targetRow = i
                        break
                    }
                }
            }
        }

        if (targetCenterX >= 0) {
            var newContentX = targetCenterX - (externalFlickable.width / 2)
            externalFlickable.contentX = Math.max(0, newContentX)
        }
    }

    Connections
    {
        target: projectController?.projectData?.taskModel ?? null
        function onRowsInserted(parent, first, last) { updateData() }
        function onRowsRemoved(parent, first, last) { updateData() }
        function onModelReset() { updateData() }
        function onDataChanged(topLeft, bottomRight, roles) { Qt.callLater(root.updateData) }
    }

    Connections
    {
        target: projectController?.projectData?.groupModel ?? null
        function onDataChanged(topLeft, bottomRight, roles) { updateData() }
        function onRowsInserted(parent, first, last) { updateData() }
        function onRowsRemoved(parent, first, last) { updateData() }
        function onCountChanged() { updateData() }
        function onModelReset() { updateData() }
        function onGroupExpandedChanged(groupId) { updateData() }
    }

    Connections
    {
        target: projectController?.projectData?.milestoneModel ?? null
        function onMilestonesChanged() { updateData() }
        function onModelReset() { updateData() }
    }

    Connections
    {
        target: projectController?.projectData?.dependencyModel ?? null
        function onRowsInserted(parent, first, last) { updateData() }
        function onRowsRemoved(parent, first, last) { updateData() }
        function onModelReset() { updateData() }
    }

    width: gridWidth
    height: contentHeight

    BackgroundCanvas
    {
        id: backgroundCanvas
        anchors.fill: parent
        z: 0
        displayStart: root.displayStart
        displayEnd: root.displayEnd
        totalDays: root.totalDays
        totalRows: root.totalRows
        dayWidth: root.dayWidth
        rowHeight: root.rowHeight
        visibleItems: root.visibleItems
    }

    MouseArea
    {
        anchors.fill: parent
        visible: root.isPure
        acceptedButtons: Qt.RightButton
        z: 50

        onClicked: function(mouse) {
            if (mouse.button !== Qt.RightButton) return
            if (!root.canEdit && !root.isLocalMode) return
            contextMenu.popup()
        }
    }

    DependencyCanvas
    {
        id: dependencyCanvas
        anchors.fill: parent
        z: 5
        displayStart: root.displayStart
        dayWidth: root.dayWidth
        rowHeight: root.rowHeight
        showDependencies: root.showDependencies
        viewMode: root.currentViewMode()
        visibleItems: root.visibleItems
    }

    Repeater
    {
        id: tasksRepeater
        model: []

        delegate: TaskRow
        {
            width: root.width
            y: index * root.rowHeight
            height: root.rowHeight

            rowData: modelData
            rowHeight: root.rowHeight
            displayStart: root.displayStart
            dayWidth: root.dayWidth
            showProgress: root.showProgress
            showComments: root.showComments
            showTaskHistory: root.showTaskHistory
            timeLineRef: root.timeLineRef
            updateCounter: root.updateCounter
            externalFlickable: root.externalFlickable
            taskContextMenu: root.taskContextMenu
            hoveredTaskId: root.hoveredTaskId
        }
    }

    Rectangle
    {
        id: bottomLine
        width: parent.width
        height: 3
        y: parent.height - height
        color: "#666666"
        z: 50
        visible: root.contentHeight > 0
    }

    Menu
    {
        id: contextMenu

        MenuItem
        {
            text: "Создать задачу"
            visible: root.canEdit
            onTriggered: {
                if (mainWindow && mainWindow.newTaskDialog) {
                    var gm = projectController.projectData.groupModel
                    var gid = gm.rowCount() > 0 ? gm.getGroupId(0) : "default"
                    mainWindow.newTaskDialog.openForGroup(gid)
                }
            }
        }

        MenuItem
        {
            text: "Создать группы по умолчанию"
            visible: root.isPure && !root.isLocalMode && root.canEdit
            onTriggered: {
                if (mainWindow && mainWindow.defaultGroupsDialog)
                    mainWindow.defaultGroupsDialog.open()
            }
        }

        MenuItem
        {
            text: "Создать группу"
            visible: !root.isLocalMode && root.canEdit
            onTriggered: {
                if (mainWindow && mainWindow.createGroupDialog)
                    mainWindow.createGroupDialog.open()
            }
        }
    }
}
