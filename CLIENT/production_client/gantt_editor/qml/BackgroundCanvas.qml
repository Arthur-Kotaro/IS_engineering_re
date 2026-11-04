import QtQuick 6.0

Canvas
{
    id: root

    property date displayStart: new Date()
    property date displayEnd: new Date()
    property int totalDays: 1
    property int totalRows: 1
    property int dayWidth: 30
    property int rowHeight: 40
    property var visibleItems: []

    function refresh()
    {
        root.requestPaint()
    }

    onPaint:
    {
        if (totalDays <= 0 || width <= 0 || height <= 0) return

        var ctx = getContext("2d")
        ctx.clearRect(0, 0, width, height)

        var showHolidays = (typeof projectController !== "undefined" && projectController
                            && projectController.settingsManager
                            && projectController.settingsManager.showHolidays)

        // 1. Розовая заливка выходных
        for (var d = 0; d < totalDays; d++)
        {
            var checkDate = new Date(displayStart)
            checkDate.setDate(checkDate.getDate() + d)
            var dow = (checkDate.getDay() + 6) % 7
            if (dow === 5 || dow === 6)
            {
                ctx.fillStyle = "#ffe0e0"
                ctx.fillRect(d * dayWidth, 0, dayWidth, height)
            }
        }

        // 1b. Голубая заливка праздников (перекрывает выходные)
        if (showHolidays)
        {
            for (var hd = 0; hd < totalDays; hd++)
            {
                var hDate = new Date(displayStart)
                hDate.setDate(hDate.getDate() + hd)

                if (projectController.settingsManager.isHoliday(hDate))
                {
                    ctx.fillStyle = "#cce5ff"
                    ctx.fillRect(hd * dayWidth, 0, dayWidth, height)
                }
            }
        }

        // 2. Вертикальные линии дней (пунктир)
        ctx.beginPath()
        ctx.setLineDash([2, 4])
        ctx.strokeStyle = "#cccccc"
        ctx.lineWidth = 1
        for (var dd = 1; dd < totalDays; dd++)
        {
            var xd = dd * dayWidth
            if (xd < width)
            {
                ctx.moveTo(xd, 0)
                ctx.lineTo(xd, height)
            }
        }
        ctx.stroke()

        // 3. Вертикальные линии недель
        ctx.beginPath()
        ctx.setLineDash([])
        ctx.strokeStyle = "#aaaaaa"
        for (var w = 1; w < totalDays / 7; w++)
        {
            var xw = w * dayWidth * 7
            if (xw < width)
            {
                ctx.moveTo(xw, 0)
                ctx.lineTo(xw, height)
            }
        }
        ctx.stroke()

        // 4. Вертикальные линии месяцев
        ctx.beginPath()
        ctx.lineWidth = 2
        ctx.strokeStyle = "#888888"
        var currentDate = new Date(displayStart)
        var xm = 0
        while (currentDate <= displayEnd)
        {
            var nextMonth = new Date(currentDate.getFullYear(), currentDate.getMonth() + 1, 1)
            var daysInMonth = Math.ceil((nextMonth - currentDate) / 86400000)
            xm += daysInMonth * dayWidth
            if (xm < width && xm > 0)
            {
                ctx.moveTo(xm, 0)
                ctx.lineTo(xm, height)
            }
            currentDate = nextMonth
        }
        ctx.stroke()

        // 5. Горизонтальные линии строк
        ctx.beginPath()
        ctx.lineWidth = 1
        ctx.strokeStyle = "#dddddd"
        for (var r = 1; r < totalRows; r++)
        {
            var y = r * rowHeight
            if (y < height)
            {
                ctx.moveTo(0, y)
                ctx.lineTo(width, y)
            }
        }
        ctx.stroke()

        // 6. Толстые горизонтальные линии границ групп
        ctx.beginPath()
        ctx.lineWidth = 2
        ctx.strokeStyle = "#888888"
        var groupRows = []
        for (var vi = 0; vi < visibleItems.length; vi++)
        {
            if (visibleItems[vi].type === "group")
                groupRows.push(vi)
        }
        for (var gi = 0; gi < groupRows.length; gi++)
        {
            var gy = groupRows[gi] * rowHeight
            if (gy < height && gy > 0)
            {
                ctx.moveTo(0, gy)
                ctx.lineTo(width, gy)
            }
        }
        ctx.stroke()
    }
}
