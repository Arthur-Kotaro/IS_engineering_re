import QtQuick 6.0
import QtQuick.Controls 6.0

Rectangle
{
    id: root
    property var rowData
    property date displayStart: new Date()
    property int dayWidth: 30
    property bool showProgress: true
    property var externalFlickable: null
    property var taskContextMenu: null
    property var parentRow: null
    property bool isResizing: false // Флаг для фиксации курсора

    HoverHandler {
        id: barHover
        onHoveredChanged: {
            if (!hovered) {
                // Мышь ушла с полосы — сбрасываем курсор в стрелку
                if (!root.isResizing && projectController)
                    projectController.setCursorShape(0)
            }
        }
    }

    signal taskDatesChanged(string taskId, date newStart, date newEnd)
    signal forecastDatesChanged(string taskId, date newStart, date newEnd)
    signal tooltipRequested(bool visible, var mouseArea, bool fromNode)
    signal tooltipHidden()

    function isForecastLocked()
    {
        if (!rowData) return false
        if (rowData.rowKind !== "forecast") return false
        var lgs = rowData.localGraphState !== undefined ? rowData.localGraphState : 0
        return (lgs === 2 || lgs === 3)
    }

    x:
    {
        if (!rowData) return 0
        var startVal = (rowData.hasDates === true) ? rowData.taskStart : rowData.forecastStart
        if (!startVal || !root.displayStart) return 0
        var daysDiff = Math.floor((new Date(startVal) - root.displayStart) / 86400000)
        return Math.max(0, daysDiff * root.dayWidth)
    }

    width:
    {
        if (!rowData) return 10
        var sVal = (rowData.hasDates === true) ? rowData.taskStart : rowData.forecastStart
        var eVal = (rowData.hasDates === true) ? rowData.taskEnd : rowData.forecastEnd
        if (!sVal || !eVal) return 10
        var daysDiff = Math.floor((new Date(eVal) - new Date(sVal)) / 86400000) + 1
        return Math.max(10, daysDiff * root.dayWidth)
    }

    height: parent.height - 8
    y: 4

    function getStatusColor()
    {
        if (!rowData) return "#FFD700"
        switch (rowData.taskStatus)
        {
            case 0: return "#FFD700"
            case 1: return "#32CD32"
            case 2: return "#FF8C00"
            case 3: return "#FF4444"
            default: return "#FFD700"
        }
    }

    function getBarColor()
    {
        if (rowData && rowData.rowKind === "forecast")
            return Qt.lighter(getStatusColor(), 1.2)
        return getStatusColor()
    }

    function getBarOpacity()
    {
        if (rowData && rowData.rowKind === "forecast") return 0.6
        return 1.0
    }

    color: getBarColor()
    opacity: getBarOpacity()
    radius: 4
    border.color: (rowData && rowData.hasConflict === true) ? "#FF4444" : "transparent"
    border.width: (rowData && rowData.hasConflict === true) ? 2 : 0

    Rectangle
    {
        id: progressFill
        visible: root.showProgress && rowData && (rowData.progressTotal || 0) > 0 && (rowData.progressCurrent || 0) >= 0 && rowData.isUnapproved !== true
        anchors.left: parent.left; anchors.leftMargin: 3; anchors.verticalCenter: parent.verticalCenter
        height: Math.max(8, parent.height * 0.75)
        width: Math.max(0, (parent.width - 6) * Math.min(1, rowData.progressCurrent / rowData.progressTotal))
        color: (rowData && rowData.rowKind === "forecast") ? Qt.lighter("#32CD32", 1.2) : "#32CD32"
        opacity: (rowData && rowData.rowKind === "forecast") ? 0.6 : 1.0
        radius: height / 2; z: 1
    }

    Text
    {
        id: progressText
        visible: root.showProgress && rowData && (rowData.progressTotal || 0) > 0 && (rowData.progressCurrent || 0) >= 0 && rowData.isUnapproved !== true && parent.width > 90
        anchors.centerIn: parent
        text: rowData ? ("Прогресс: " + rowData.progressCurrent + "/" + rowData.progressTotal) : ""
        font.pixelSize: 11; color: "#222222"; z: 2
    }

    Canvas
    {
        id: stripesCanvas
        anchors.fill: parent
        visible: rowData && rowData.isUnapproved === true
        onPaint:
        {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            if (!rowData || rowData.isUnapproved !== true || width <= 0 || height <= 0) return
            var stripeWidth = 12; var gap = 12; var step = stripeWidth + gap
            ctx.strokeStyle = "#ffffff"; ctx.lineWidth = stripeWidth; ctx.beginPath()
            var diagonal = height
            for (var i = -diagonal; i < width + diagonal; i += step) {
                ctx.moveTo(i, height); ctx.lineTo(i + diagonal, 0)
            }
            ctx.stroke()
        }
        onWidthChanged: requestPaint(); onHeightChanged: requestPaint(); onVisibleChanged: if (visible) requestPaint()
    }

    function updateDates()
    {
        var newStartDays = Math.round(x / root.dayWidth)
        var newStart = new Date(root.displayStart)
        newStart.setDate(newStart.getDate() + newStartDays)

        var newEndDays = Math.round((x + root.width) / root.dayWidth)
        var newEnd = new Date(root.displayStart)
        newEnd.setDate(newEnd.getDate() + newEndDays - 1)

        if (newStart < newEnd)
        {
            if (rowData && rowData.rowKind === "forecast") root.forecastDatesChanged(rowData.taskId, newStart, newEnd)
            else root.taskDatesChanged(rowData.taskId, newStart, newEnd)
        }
    }

    // Полоса (Drag)
    MouseArea
    {
        id: moveArea
        anchors.fill: parent
        anchors.rightMargin: 15 // Оставляем место для resizeHandle
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        property bool suppressClick: false
        property bool showTooltip: false

        Timer
        {
            id: tooltipDelay
            interval: 500; repeat: false
            onTriggered: { moveArea.showTooltip = true; root.tooltipRequested(true, moveArea, false) }
        }

        onEntered: {
            if (!root.isResizing) {
                if (!(projectController && projectController.settingsManager.editingLocked) && 
                    !root.isForecastLocked() && !(rowData && rowData.isCompleted)) {
                    if (projectController) projectController.setCursorShape(2) // OpenHand
                }
            }
            tooltipDelay.restart()
        }
        onExited: { 
            // Убираем setCursorShape(0), чтобы не сбивать курсор при переходе в resizeArea
            tooltipDelay.stop(); moveArea.showTooltip = false; root.tooltipHidden()
        }
        
        onPositionChanged: function(mouse) {
            // Принудительно ставим руку, если мы не тащим и не ресайзим
            if (!drag.active && !root.isResizing) {
                if (!(projectController && projectController.settingsManager.editingLocked) && 
                    !root.isForecastLocked() && !(rowData && rowData.isCompleted)) {
                    if (projectController) projectController.setCursorShape(2) // OpenHand
                }
            }
            
            if (!drag.active) return
            if (root.isForecastLocked()) { drag.target = null; return }
            if (projectController && projectController.settingsManager.editingLocked) { drag.target = null; return }
            if (drag.target !== null && drag.target !== undefined) {
                suppressClick = true
                parent.x = Math.round(parent.x / root.dayWidth) * root.dayWidth
            }
        }

        onPressed: function(mouse)
        {
            if (mouse.button === Qt.RightButton) { drag.target = null; suppressClick = false; return }
            if (root.isForecastLocked()) {
                if (projectController) projectController.notifyEditingLocked()
                drag.target = null
                return
            }
            if (projectController && projectController.settingsManager.editingLocked) { projectController.notifyEditingLocked(); drag.target = null; return }
            if (rowData && rowData.isCompleted) { if (projectController) projectController.notifyTaskCompleted(); drag.target = null; return }

            drag.target = parent
            drag.axis = Drag.XAxis
            drag.minimumX = 0
            drag.maximumX = root.parentRow.width - parent.width
            if (root.externalFlickable) root.externalFlickable.interactive = false
        }

        onReleased: function(mouse)
        {
            if (root.externalFlickable) root.externalFlickable.interactive = true
            if (drag.active) { drag.target = null; parent.updateDates() }
            if (projectController) projectController.setCursorShape(2) // OpenHand
        }

        onClicked: function(mouse)
        {
            if (mouse.button === Qt.RightButton && !suppressClick) {
                if (rowData && root.taskContextMenu) { root.taskContextMenu.taskId = rowData.taskId; root.taskContextMenu.popup() }
            }
        }
    }

    // Правый край (Resize)
    Rectangle
    {
        id: resizeHandle
        width: 15
        height: parent.height
        anchors.right: parent.right
        color: resizeArea.containsMouse ? Qt.darker(root.color, 1.5) : "transparent"
        radius: 2
        z: 10

        MouseArea
        {
            id: resizeArea
            anchors.fill: parent
            hoverEnabled: true

            property real startWidth: 0
            property real startX: 0

            onEntered: {
                if (root.isResizing) return
                if (!(projectController && projectController.settingsManager.editingLocked) && 
                    !root.isForecastLocked() && !(rowData && rowData.isCompleted)) {
                    if (projectController) projectController.setCursorShape(1) // SizeHor
                }
            }
            onExited: {
                // Убираем setCursorShape(0), чтобы не сбивать курсор при переходе в moveArea
            }

            onPositionChanged: function(mouse)
            {
                // Принудительно ставим SizeHor, если мы не тащим
                if (!pressed && !root.isResizing) {
                    if (!(projectController && projectController.settingsManager.editingLocked) && 
                        !root.isForecastLocked() && !(rowData && rowData.isCompleted)) {
                        if (projectController) projectController.setCursorShape(1) // SizeHor
                    }
                }

                if (root.isForecastLocked()) return
                if (projectController && projectController.settingsManager.editingLocked) return
                if (rowData && rowData.isCompleted) return

                if (pressed)
                {
                    var currentX = mapToItem(root.parentRow, mouse.x, mouse.y).x
                    var delta = currentX - startX
                    var newWidth = startWidth + delta
                    newWidth = Math.max(root.dayWidth, Math.round(newWidth / root.dayWidth) * root.dayWidth)
                    root.width = newWidth
                }
            }

            onPressed: function(mouse)
            {
                if (root.isForecastLocked()) {
                    if (projectController) projectController.notifyEditingLocked()
                    return
                }
                if (projectController && projectController.settingsManager.editingLocked) { projectController.notifyEditingLocked(); return }
                if (rowData && rowData.isCompleted) { if (projectController) projectController.notifyTaskCompleted(); return }

                root.isResizing = true // <-- УСТАНОВИТЬ ФЛАГ
                if (projectController) projectController.setCursorShape(1) // SizeHor
                
                if (root.externalFlickable) root.externalFlickable.interactive = false
                startWidth = root.width
                startX = mapToItem(root.parentRow, mouse.x, mouse.y).x
                root.width = startWidth
            }

            onReleased: function(mouse)
            {
                root.isResizing = false // <-- СБРОСИТЬ ФЛАГ
                if (projectController) projectController.setCursorShape(1) // SizeHor
                
                if (root.externalFlickable) root.externalFlickable.interactive = true
                if (projectController && projectController.settingsManager.editingLocked) return
                if (rowData && rowData.isCompleted) return
                root.updateDates()
            }
        }
    }
}
