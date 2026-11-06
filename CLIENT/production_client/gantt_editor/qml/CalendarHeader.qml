import QtQuick 6.0
import QtQuick.Controls 6.0

Rectangle
{
    id: root
    property alias milestoneBar: milestoneBar
    color: "#f8f8f8"
    width: parent?.width || 1000

    property int rowHeight: 40
    height: columnContent.height

    property date firstMilestoneDate: new Date()
    property date lastMilestoneDate: new Date()

    property date displayStart: new Date()
    property date displayEnd: new Date()

    readonly property bool isLocalMode: {
        return projectController && projectController.projectData
               && projectController.projectData.graphKind === 1
    }

    readonly property bool showHolidays: {
        return projectController && projectController.settingsManager
               && projectController.settingsManager.showHolidays
    }

    function isValidDate(d)
    {
        return d !== undefined && d !== null && !isNaN(new Date(d).getTime())
    }

    function isHolidayDate(date)
    {
        if (!showHolidays) return false
        if (!projectController || !projectController.settingsManager) return false
        return projectController.settingsManager.isHoliday(date)
    }

    function getSecondSundayAfter(date)
    {
        var d = new Date(date)
        if (isNaN(d.getTime())) return new Date()
        while (d.getDay() !== 0) d.setDate(d.getDate() + 1)
        d.setDate(d.getDate() + 14)
        d.setHours(23, 59, 59, 999)
        return d
    }

    function getMondayBefore(date, weeks)
    {
        var d = new Date(date)
        while (d.getDay() !== 1) d.setDate(d.getDate() - 1)
        d.setDate(d.getDate() - weeks * 7)
        d.setHours(0, 0, 0, 0)
        return d
    }

    function computeLocalDisplayRange(linkedStart, linkedEnd, earliest, latest)
    {
        var s = new Date(linkedStart)
        var e = new Date(linkedEnd)

        var durationDays = Math.floor((e - s) / 86400000) + 1
        if (durationDays < 30)
        {
            var ms = new Date(s.getFullYear(), s.getMonth(), 1)
            ms.setDate(ms.getDate() - 14)
            ms.setHours(0, 0, 0, 0)
            var me = new Date(e.getFullYear(), e.getMonth() + 1, 0)
            me.setDate(me.getDate() + 14)
            me.setHours(23, 59, 59, 999)
            s = ms
            e = me
        }
        else
        {
            s.setDate(s.getDate() - 14)
            s.setHours(0, 0, 0, 0)
            e.setDate(e.getDate() + 14)
            e.setHours(23, 59, 59, 999)
        }

        if (isValidDate(earliest) && earliest < s) s = new Date(earliest)
        if (isValidDate(latest) && latest > e) e = new Date(latest)

        while (s.getDay() !== 1) s.setDate(s.getDate() - 1)

        return { start: s, end: e }
    }

    function updateDisplayRange()
    {
        var localMode = (projectController && projectController.projectData
        && projectController.projectData.graphKind === 1)

        if (localMode)
        {
            var linkedStart = projectController.projectData.getLinkedTargetStart()
            var linkedEnd = projectController.projectData.getLinkedTargetEnd()

            var earliest = projectController.projectData.getEarliestDate()
            var latest = projectController.projectData.getLatestDate()

            var s, e

            if (isValidDate(linkedStart) && isValidDate(linkedEnd))
            {
                var range = computeLocalDisplayRange(linkedStart, linkedEnd, earliest, latest)
                s = range.start
                e = range.end
            }
            else
            {
                var now = new Date()
                s = new Date(now.getFullYear(), now.getMonth() - 1, 1)
                e = new Date(now.getFullYear(), now.getMonth() + 2, 0)
                s.setHours(0, 0, 0, 0)
                e.setHours(23, 59, 59, 999)
                if (isValidDate(earliest) && earliest < s) s = new Date(earliest)
                    if (isValidDate(latest) && latest > e) e = new Date(latest)
                        while (s.getDay() !== 1) s.setDate(s.getDate() - 1)
            }

            displayStart = s
            displayEnd = e
            return
        }

        var earliest = projectController?.projectData?.getEarliestDate()
        var latest = projectController?.projectData?.getLatestDate()
        if (!earliest || !latest) return
            var start = getMondayBefore(earliest, 4)
            var end = getSecondSundayAfter(latest)
            if (displayStart.toDateString() !== start.toDateString()) displayStart = start
                if (displayEnd.toDateString() !== end.toDateString()) displayEnd = end
    }

    property int dayWidth: projectController && projectController.settingsManager.zoomLevel === 1 ? 10 : 30
    onDayWidthChanged: { if (milestoneBar) milestoneBar.dayWidth = dayWidth }
    property int totalDays: Math.max(1, Math.floor((displayEnd - displayStart) / 86400000) + 1)
    property real contentWidth: totalDays * dayWidth

    property var yearData: []
    property var monthData: []
    property var weekData: []
    property var dayNumbers: []

    signal calendarWidthChanged()

    onContentWidthChanged: calendarWidthChanged()

    function rebuildData()
    {
        if (totalDays <= 0) return

        var years = [], months = [], weeks = [], dayNums = []
        var currentDate = new Date(displayStart)
        for (var i = 0; i < totalDays; i++)
        {
            dayNums.push(currentDate.getDate())
            currentDate.setDate(currentDate.getDate() + 1)
        }

        currentDate = new Date(displayStart)
        var currentYear = currentDate.getFullYear()
        var yearStartIdx = 0, yearDays = 0
        var currentMonth = currentDate.getMonth()
        var monthStartIdx = 0, monthDays = 0

        for (var j = 0; j < totalDays; j++)
        {
            var year = currentDate.getFullYear()
            var month = currentDate.getMonth()

            if (year !== currentYear)
            {
                years.push({year: currentYear, startIdx: yearStartIdx, days: yearDays})
                currentYear = year; yearStartIdx = j; yearDays = 0
            }
            if (month !== currentMonth)
            {
                months.push({name: ["Янв","Фев","Мар","Апр","Май","Июн","Июл","Авг","Сен","Окт","Ноя","Дек"][currentMonth],
                            startIdx: monthStartIdx, days: monthDays})
                currentMonth = month; monthStartIdx = j; monthDays = 0
            }
            yearDays++; monthDays++
            currentDate.setDate(currentDate.getDate() + 1)
        }
        years.push({year: currentYear, startIdx: yearStartIdx, days: yearDays})
        months.push({name: ["Янв","Фев","Мар","Апр","Май","Июн","Июл","Авг","Сен","Окт","Ноя","Дек"][currentMonth],
                    startIdx: monthStartIdx, days: monthDays})

        var weekStart = new Date(displayStart)
        for (var w = 0; w < Math.ceil(totalDays / 7); w++)
        {
            var firstThu = new Date(weekStart)
            firstThu.setDate(firstThu.getDate() + 3 - ((firstThu.getDay() + 6) % 7))
            var firstJan = new Date(firstThu.getFullYear(), 0, 4)
            var wn = 1 + Math.round(((firstThu - firstJan) / 86400000 - 3 + ((firstJan.getDay() + 6) % 7)) / 7)
            weeks.push({num: wn, startIdx: w * 7})
            weekStart.setDate(weekStart.getDate() + 7)
        }

        yearData = years; monthData = months; weekData = weeks; dayNumbers = dayNums
    }

    function refresh()
    {
        updateDisplayRange()
        rebuildData()
    }

    Component.onCompleted: refresh()

    Connections
    {
        target: projectController
        function onProjectDataChanged() { refresh() }
        function onProjectLoaded() { refresh() }
    }

    Connections
    {
        target: projectController?.settingsManager ?? null
        function onShowHolidaysChanged() { refresh() }
        function onHolidaysChanged() { refresh() }
    }

    Connections
    {
        target: projectController?.projectData ?? null
        function onGraphKindChanged() { refresh() }
        function onDataCleared() { refresh() }
    }

    Connections
    {
        target: projectController?.projectData?.milestoneModel ?? null
        function onMilestonesChanged() { refresh() }
        function onModelReset() { refresh() }
    }

    Connections
    {
        target: projectController?.projectData?.taskModel ?? null
        function onRowsInserted(parent, first, last) { refresh() }
        function onRowsRemoved(parent, first, last) { refresh() }
        function onDataChanged(topLeft, bottomRight, roles) { refresh() }
    }

    onDisplayStartChanged: rebuildData()
    onDisplayEndChanged: rebuildData()

    Column
    {
        id: columnContent
        spacing: 0

        Rectangle
        {
            width: contentWidth; height: rowHeight; color: "#e0e0e0"; border.color: "#888888"; border.width: 1
            Row { Repeater { model: yearData
                Rectangle { x: modelData.startIdx * dayWidth; width: modelData.days * dayWidth; height: rowHeight; border.color: "#aaaaaa"; border.width: 1; color: "transparent"
                    Text { text: modelData.year; anchors.centerIn: parent; font.bold: true; font.pixelSize: 14 } } } }
        }

        Rectangle
        {
            width: contentWidth; height: rowHeight; color: "#e8e8e8"; border.color: "#888888"; border.width: 1
            Row { Repeater { model: monthData
                Rectangle { x: modelData.startIdx * dayWidth; width: modelData.days * dayWidth; height: rowHeight; border.color: "#aaaaaa"; border.width: 1; color: "transparent"
                    Text { text: modelData.name; anchors.centerIn: parent; font.pixelSize: 12 } } } }
        }

        Rectangle
        {
            width: contentWidth; height: rowHeight; color: "#f0f0f0"; border.color: "#aaaaaa"; border.width: 1
            Row { Repeater { model: weekData
                Rectangle { x: modelData.startIdx * dayWidth; width: 7 * dayWidth; height: rowHeight; border.color: "#aaaaaa"; border.width: 1; color: "transparent"
                    Text { text: "КН" + modelData.num; anchors.centerIn: parent; font.pixelSize: 10 } } } }
        }

        Rectangle
        {
            width: contentWidth; height: rowHeight; color: "#f8f8f8"; border.color: "#aaaaaa"; border.width: 1
            Row
            {
                Repeater
                {
                    model: totalDays

                    delegate: Rectangle
                    {
                        x: index * dayWidth
                        width: dayWidth
                        height: rowHeight
                        border.color: "#aaaaaa"
                        border.width: 1

                        property date cellDate: {
                            var d = new Date(displayStart)
                            d.setDate(d.getDate() + index)
                            return d
                        }
                        property bool cellIsHoliday: root.isHolidayDate(cellDate)
                        property bool cellIsWeekend: {
                            var dow = ((displayStart.getDay() + index) % 7 + 6) % 7
                            return (dow === 5 || dow === 6)
                        }

                        color: {
                            if (cellIsHoliday) return "#cce5ff"
                            if (cellIsWeekend) return "#ffe0e0"
                            return (index % 2 === 0 ? "#ffffff" : "#f8f8f8")
                        }

                        MouseArea
                        {
                            id: dayMouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            acceptedButtons: Qt.NoButton
                        }

                        ToolTip
                        {
                            visible: dayMouseArea.containsMouse && parent.cellIsHoliday
                            delay: 400
                            text: {
                                if (!parent.cellIsHoliday) return ""
                                if (!projectController || !projectController.settingsManager) return ""
                                return projectController.settingsManager.holidaysTooltip(parent.cellDate)
                            }
                        }

                        Column
                        {
                            anchors.centerIn: parent
                            spacing: 2
                            visible: root.dayWidth >= 15

                            Text
                            {
                                text: ["Пн","Вт","Ср","Чт","Пт","Сб","Вс"][((displayStart.getDay() + index) % 7 + 6) % 7]
                                anchors.horizontalCenter: parent.horizontalCenter
                                font.pixelSize: 10
                                font.bold: true
                                color: parent.parent.cellIsHoliday ? "#cc0000" : "#000000"
                            }
                            Text
                            {
                                text: root.dayNumbers[index] || ""
                                anchors.horizontalCenter: parent.horizontalCenter
                                font.pixelSize: 11
                                font.bold: parent.parent.cellIsHoliday
                                color: parent.parent.cellIsHoliday ? "#cc0000" : "#000000"
                            }
                        }
                    }
                }
            }
        }

        MilestoneBar
        {
            id: milestoneBar
            visible: !root.isLocalMode
            width: contentWidth
            height: root.isLocalMode ? 0 : root.rowHeight
            milestonesModel: projectController?.projectData?.milestoneModel
            startDate: displayStart; dayWidth: dayWidth
        }

        Rectangle
        {
            width: contentWidth
            height: rowHeight
            color: "#f5f5f5"
            border.color: "#dddddd"

            border.width: 1
            Row
            {
                anchors.fill: parent
                anchors.leftMargin: 10
                spacing: 20
                Text
                {
                    text: "Создан: " + (projectController?.projectData?.creationDateTime?.toLocaleString() || "не указано")
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: 11
                }
                Text
                {
                    text: "Изменён: " + (projectController?.projectData?.lastModifiedDateTime?.toLocaleString() || "не указано")
                    anchors.verticalCenter: parent.verticalCenter
                    font.pixelSize: 11
                }
            }
        }
    }
}
