import QtQuick 6.0

Canvas
{
    id: root

    property var visibleItems: []
    property date displayStart: new Date()
    property int dayWidth: 30
    property int rowHeight: 40
    property bool showDependencies: true
    property int viewMode: 0

    function refresh()
    {
        root.requestPaint()
    }

    function getRowIndicesForTask(taskId)
    {
        var result = { targetRow: -1, forecastRow: -1 }
        for (var i = 0; i < visibleItems.length; i++)
        {
            var it = visibleItems[i]
            if (it.type !== "task" || it.taskId !== taskId) continue
            if (it.rowKind === "target") result.targetRow = i
            else if (it.rowKind === "forecast") result.forecastRow = i
        }
        return result
    }

    function isValidDate(d)
    {
        return d !== undefined && d !== null && !isNaN(new Date(d).getTime())
    }

    function getDependencyAnchorX(rowIndex, isOutgoing)
    {
        if (rowIndex < 0 || rowIndex >= visibleItems.length) return -1
        var item = visibleItems[rowIndex]
        if (!item) return -1

        if (item.hasDates === true)
        {
            if (isOutgoing)
            {
                var endDays = Math.floor((new Date(item.taskEnd) - displayStart) / 86400000)
                return endDays * dayWidth + dayWidth
            }
            else
            {
                var startDays = Math.floor((new Date(item.taskStart) - displayStart) / 86400000)
                return startDays * dayWidth
            }
        }
        else if (item.isUnapproved === true)
        {
            if (isOutgoing)
            {
                var feDays = Math.floor((new Date(item.forecastEnd) - displayStart) / 86400000)
                return feDays * dayWidth + dayWidth
            }
            else
            {
                var fsDays = Math.floor((new Date(item.forecastStart) - displayStart) / 86400000)
                return fsDays * dayWidth
            }
        }
        else
        {
            var nodeCenter = item.nodeX || 0
            return isOutgoing ? nodeCenter + 15 : nodeCenter - 15
        }
    }

    function drawDependencyLine(ctx, fromRow, toRow)
    {
        if (fromRow < 0 || toRow < 0) return

        var fromX = getDependencyAnchorX(fromRow, true)
        var toX = getDependencyAnchorX(toRow, false)
        if (fromX < 0 || toX < 0) return

        var fromY = fromRow * rowHeight + rowHeight / 2
        var toY = toRow * rowHeight + rowHeight / 2

        ctx.beginPath()
        ctx.strokeStyle = "#9966cc"
        ctx.lineWidth = 2
        ctx.moveTo(fromX, fromY)
        ctx.lineTo(fromX + 10, fromY)
        ctx.lineTo(toX - 10, toY)
        ctx.lineTo(toX, toY)
        ctx.stroke()
    }

    onPaint:
    {
        if (width <= 0 || height <= 0) return

        var ctx = getContext("2d")
        ctx.clearRect(0, 0, width, height)

        if (!showDependencies) return
        if (!projectController || !projectController.projectData || !projectController.projectData.dependencyModel) return

        var depModel = projectController.projectData.dependencyModel

        for (var di = 0; di < depModel.rowCount(); di++)
        {
            var idx = depModel.index(di, 0)
            var predId = depModel.data(idx, Qt.UserRole + 2)
            var succId = depModel.data(idx, Qt.UserRole + 3)
            if (!predId || !succId) continue

            var predTask = projectController.projectData.taskModel.getTask(predId)
            var succTask = projectController.projectData.taskModel.getTask(succId)
            if (!predTask || !succTask) continue

            var predCompleted = (predTask.status === 1)
            var succCompleted = (succTask.status === 1)

            var predRows = getRowIndicesForTask(predId)
            var succRows = getRowIndicesForTask(succId)

            if (viewMode === 0)
            {
                drawDependencyLine(ctx, predRows.targetRow, succRows.targetRow)
            }
            else if (viewMode === 1)
            {
                var predSrc = predCompleted ? predRows.targetRow : predRows.forecastRow
                var succDst = succCompleted ? succRows.targetRow : succRows.forecastRow
                if (predSrc < 0) predSrc = predRows.targetRow
                if (succDst < 0) succDst = succRows.targetRow
                drawDependencyLine(ctx, predSrc, succDst)
            }
            else
            {
                drawDependencyLine(ctx, predRows.targetRow, succRows.targetRow)

                if (succRows.forecastRow >= 0)
                {
                    var predForecastSrc = predCompleted ? predRows.targetRow : predRows.forecastRow
                    if (predForecastSrc >= 0)
                        drawDependencyLine(ctx, predForecastSrc, succRows.forecastRow)
                }
            }
        }
    }
}
