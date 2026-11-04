import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0
import "utils/DateUtils.js" as DateUtils

Popup
{
    id: root

    property date selectedDate: new Date()
    property date minDate: new Date(1900, 0, 1)
    property date maxDate: new Date(2100, 11, 31)
    property color highlightColor: "#0078d7"

    signal dateSelected(date d)

    width: 380
    height: 360
    modal: true
    focus: true
    anchors.centerIn: Overlay.overlay

    property int calendarYear: selectedDate.getFullYear()
    property int calendarMonth: selectedDate.getMonth()
    property var calendarModel: []

    onSelectedDateChanged:
    {
        calendarYear = selectedDate.getFullYear()
        calendarMonth = selectedDate.getMonth()
        updateCalendarModel()
    }

    function updateCalendarModel()
    {
        var firstDay = new Date(calendarYear, calendarMonth, 1)
        var startWeekday = firstDay.getDay()
        var startIndex = (startWeekday === 0) ? 6 : startWeekday - 1
        var daysInMonth = new Date(calendarYear, calendarMonth + 1, 0).getDate()
        var result = []
        for (var i = 0; i < startIndex; i++) result.push("")
        for (var j = 1; j <= daysInMonth; j++) result.push(j)
        var remaining = (7 - (result.length % 7)) % 7
        for (var k = 0; k < remaining; k++) result.push("")
        calendarModel = result
    }

    function isOutOfRange(y, m, d)
    {
        var dt = new Date(y, m, d)
        if (DateUtils.isValidDate(minDate) && dt < DateUtils.startOfDay(minDate)) return true
        if (DateUtils.isValidDate(maxDate) && dt > DateUtils.startOfDay(maxDate)) return true
        return false
    }

    onOpened:
    {
        calendarYear = selectedDate.getFullYear()
        calendarMonth = selectedDate.getMonth()
        updateCalendarModel()
    }

    Rectangle
    {
        anchors.fill: parent
        color: "white"
        border.color: "#cccccc"
        radius: 4

        ColumnLayout
        {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 10

            RowLayout
            {
                Layout.fillWidth: true

                Button
                {
                    text: "<<"
                    font.pixelSize: 14
                    Layout.preferredWidth: 45
                    onClicked: { root.calendarYear -= 1; root.updateCalendarModel() }
                }
                Button
                {
                    text: "<"
                    font.pixelSize: 14
                    Layout.preferredWidth: 40
                    onClicked:
                    {
                        if (root.calendarMonth === 0) { root.calendarMonth = 11; root.calendarYear -= 1 }
                        else root.calendarMonth -= 1
                        root.updateCalendarModel()
                    }
                }
                Label
                {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    font.pixelSize: 16
                    font.bold: true
                    text:
                    {
                        var months = ["Январь", "Февраль", "Март", "Апрель", "Май", "Июнь",
                                     "Июль", "Август", "Сентябрь", "Октябрь", "Ноябрь", "Декабрь"]
                        return months[root.calendarMonth] + " " + root.calendarYear
                    }
                }
                Button
                {
                    text: ">"
                    font.pixelSize: 14
                    Layout.preferredWidth: 40
                    onClicked:
                    {
                        if (root.calendarMonth === 11) { root.calendarMonth = 0; root.calendarYear += 1 }
                        else root.calendarMonth += 1
                        root.updateCalendarModel()
                    }
                }
                Button
                {
                    text: ">>"
                    font.pixelSize: 14
                    Layout.preferredWidth: 45
                    onClicked: { root.calendarYear += 1; root.updateCalendarModel() }
                }
            }

            Rectangle
            {
                Layout.fillWidth: true
                height: 25
                color: "#f0f0f0"
                Row
                {
                    spacing: 2
                    Repeater
                    {
                        model: ["Пн", "Вт", "Ср", "Чт", "Пт", "Сб", "Вс"]
                        Label
                        {
                            width: 46
                            height: 25
                            text: modelData
                            font.pixelSize: 13
                            font.bold: true
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
            }

            GridView
            {
                Layout.fillWidth: true
                Layout.fillHeight: true
                cellWidth: 48
                cellHeight: 40

                model: root.calendarModel
                delegate: Rectangle
                {
                    width: 46
                    height: 38
                    radius: 3
                    color:
                    {
                        if (modelData === "") return "transparent"
                        if (root.isOutOfRange(root.calendarYear, root.calendarMonth, modelData)) return "transparent"
                        var cellDate = new Date(root.calendarYear, root.calendarMonth, modelData)
                        if (cellDate.toDateString() === root.selectedDate.toDateString()) return root.highlightColor
                        return "transparent"
                    }

                    Text
                    {
                        anchors.centerIn: parent
                        text: modelData
                        font.pixelSize: 14
                        color:
                        {
                            if (modelData === "") return "transparent"
                            if (root.isOutOfRange(root.calendarYear, root.calendarMonth, modelData)) return "#cccccc"
                            var cellDate = new Date(root.calendarYear, root.calendarMonth, modelData)
                            if (cellDate.toDateString() === root.selectedDate.toDateString()) return "white"
                            return "black"
                        }
                    }

                    MouseArea
                    {
                        anchors.fill: parent
                        enabled: modelData !== ""
                                 && !root.isOutOfRange(root.calendarYear, root.calendarMonth, modelData)
                        onClicked:
                        {
                            var newDate = new Date(root.calendarYear, root.calendarMonth, modelData)
                            root.selectedDate = newDate
                            root.dateSelected(newDate)
                            root.close()
                        }
                    }
                }
            }
        }
    }
}
