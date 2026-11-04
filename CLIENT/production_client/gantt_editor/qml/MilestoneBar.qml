import QtQuick 6.0
import QtQuick.Controls 6.0

Rectangle
{
    id: root
    property alias canvas: milestoneCanvas
    color: "#f0f0f0"
    border.color: "#aaaaaa"
    border.width: 1

    property int dayWidth: 30
    onDayWidthChanged:
    {
        //console.log("MilestoneBar dayWidth changed:", dayWidth)
        updateMilestones()
        milestoneCanvas.requestPaint()
    }
    property var milestonesModel: null
    property date startDate: new Date()
    property var milestones: []
    property bool showRescheduled: true
    onShowRescheduledChanged: milestoneCanvas.requestPaint()

    function updateMilestones()
    {
        if (milestonesModel)
        {
            var allMilestones = milestonesModel.getAllMilestones()
            var result = []
            for (var i = 0; i < allMilestones.length; i++)
            {
                var ms = allMilestones[i]
                result.push(ms)
                if (ms.rescheduleHistory && ms.rescheduleHistory.length > 0)
                {
                    for (var j = 0; j < ms.rescheduleHistory.length; j++)
                    {
                        var dateStr = ms.rescheduleHistory[j]
                        var parts = dateStr.split(".")
                        if (parts.length === 3)
                        {
                            var oldDate = new Date(parseInt(parts[2]), parseInt(parts[1]) - 1, parseInt(parts[0]))
                            var historyItem = {
                                milestoneId: ms.milestoneId,
                                abbreviation: ms.abbreviation,
                                fullName: ms.fullName,
                                tooltip: ms.tooltip,
                                plannedDate: oldDate,
                                status: 2,
                                color: "#888888",
                                actualDate: ms.actualDate
                            }
                            result.push(historyItem)
                        }
                    }
                }
            }
            milestones = result
            milestoneCanvas.requestPaint()
        }
        else
        {
            milestones = []
        }
    }

    Component.onCompleted: { updateMilestones(); if (!dayWidth) dayWidth = 30 }

    Connections
    {
        target: milestonesModel || null
        enabled: milestonesModel !== null
        function onModelReset() { updateMilestones() }
        function onDataChanged() { updateMilestones() }
        function onMilestonesChanged() { updateMilestones() }
        function onMilestoneStatusChanged() { updateMilestones() }
    }

    Canvas
    {
        id: milestoneCanvas
        anchors.fill: parent
        z: 1

        onPaint:
        {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            for (var i = 0; i < milestones.length; i++)
            {
                var ms = milestones[i]
                if (ms.color === "#888888" && !root.showRescheduled) continue

                var plannedDate = new Date(ms.plannedDate)
                var daysDiff = Math.floor((plannedDate - root.startDate) / (1000 * 60 * 60 * 24))
                var cx = daysDiff * root.dayWidth + root.dayWidth / 2
                var cy = 15

                ctx.save()
                ctx.translate(cx, cy)
                ctx.rotate(Math.PI / 4)

                if (ms.color) ctx.fillStyle = ms.color
                else if (ms.status === 1) ctx.fillStyle = "#32CD32"
                else if (ms.status === 2) ctx.fillStyle = "#FFD700"
                else ctx.fillStyle = "#FFD700"

                ctx.fillRect(-6, -6, 12, 12)
                ctx.restore()

                ctx.fillStyle = "#000000"
                ctx.font = "bold 10px sans-serif"
                ctx.textAlign = "center"
                ctx.fillText(ms.abbreviation || "", cx, cy + 16)
            }
        }
    }

    MouseArea
    {
        id: milestoneMouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.RightButton

        onClicked: function(mouse)
        {
            if (mouse.button === Qt.RightButton)
            {
                var mx = mouse.x
                for (var i = 0; i < milestones.length; i++)
                {
                    var ms = milestones[i]

                    // Исторические (серые) вехи не должны реагировать на клики
                    if (ms.color === "#888888") continue

                        var plannedDate = new Date(ms.plannedDate)
                        var daysDiff = Math.floor((plannedDate - root.startDate) / (1000 * 60 * 60 * 24))
                        var cx = daysDiff * root.dayWidth + root.dayWidth / 2
                        var cy = 8

                        if (Math.abs(mx - cx) < 12)
                        {
                            milestoneContextMenu.milestoneId = ms.milestoneId
                            milestoneContextMenu.currentDate = ms.plannedDate
                            milestoneContextMenu.currentStatus = ms.status !== undefined ? ms.status : 0
                            milestoneContextMenu.currentAbbreviation = ms.abbreviation
                            milestoneContextMenu.currentFullName = ms.fullName
                            milestoneContextMenu.currentActualDate = ms.actualDate
                            milestoneContextMenu.popup()
                            return
                        }
                }
            }
        }
    }

    Dialog
    {
        id: confirmMilestoneDialog
        title: "Подтверждение"
        width: 450
        height: 180
        modal: true
        standardButtons: Dialog.NoButton
        anchors.centerIn: Overlay.overlay

        property string milestoneId: ""
        property string milestoneName: ""

        Column
        {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 20

            Label
            {
                width: parent.width
                text: "Вы готовы пройти веху \"" + confirmMilestoneDialog.milestoneName + "\"?"
                wrapMode: Text.WordWrap
                font.pixelSize: 14
            }

            Row
            {
                anchors.right: parent.right
                spacing: 10

                Button
                {
                    text: "Нет"
                    width: 80
                    height: 35
                    onClicked: confirmMilestoneDialog.close()
                }

                Button
                {
                    text: "Да"
                    width: 80
                    height: 35
                    onClicked:
                    {
                        if (confirmMilestoneDialog.milestoneId && milestonesModel)
                        {
                            milestonesModel.setMilestoneCompleted(confirmMilestoneDialog.milestoneId)
                            updateMilestones()
                        }
                        confirmMilestoneDialog.close()
                    }
                }
            }
        }
    }

    Dialog
    {
        id: rescheduleMilestoneDialog
        title: "Перенос даты вехи"
        width: 400
        height: 250
        modal: true
        standardButtons: Dialog.NoButton
        anchors.centerIn: Overlay.overlay

        property string milestoneId: ""
        property date currentDate: new Date()

        Column
        {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 15

            Label
            {
                text: "Текущая дата: " + Qt.formatDateTime(rescheduleMilestoneDialog.currentDate, "dd.MM.yyyy")
                font.pixelSize: 13
            }

            Label
            {
                text: "Новая дата прохождения вехи:"
                font.pixelSize: 13
            }

            TextField
            {
                id: newDateField
                width: parent.width
                height: 40
                text: Qt.formatDateTime(rescheduleMilestoneDialog.currentDate, "dd.MM.yyyy")
                font.pixelSize: 14
                horizontalAlignment: Text.AlignHCenter
                Keys.onReturnPressed: { rescheduleMilestoneDialog.applyReschedule() }
                onEditingFinished:
                {
                    var parts = text.split(".")
                    if (parts.length === 3)
                    {
                        var d = new Date(parseInt(parts[2]), parseInt(parts[1]) - 1, parseInt(parts[0]))
                        if (!isNaN(d.getTime()))
                        {
                            rescheduleMilestoneDialog.currentDate = d
                            text = Qt.formatDateTime(d, "dd.MM.yyyy")
                        }
                        else
                        {
                            text = Qt.formatDateTime(rescheduleMilestoneDialog.currentDate, "dd.MM.yyyy")
                        }
                    }
                    else
                    {
                        text = Qt.formatDateTime(rescheduleMilestoneDialog.currentDate, "dd.MM.yyyy")
                    }
                }
            }

            Row
            {
                anchors.right: parent.right
                spacing: 10

                Button
                {
                    text: "Отмена"
                    width: 80
                    height: 35
                    onClicked: rescheduleMilestoneDialog.close()
                }

                Button
                {
                    text: "Перенести"
                    width: 90
                    height: 35
                    onClicked: rescheduleMilestoneDialog.applyReschedule()
                }
            }
        }

        function applyReschedule()
        {
            var parts = newDateField.text.split(".")
            if (parts.length === 3)
            {
                var d = new Date(parseInt(parts[2]), parseInt(parts[1]) - 1, parseInt(parts[0]))
                if (!isNaN(d.getTime()))
                {
                    rescheduleMilestoneDialog.currentDate = d
                }
            }
            if (rescheduleMilestoneDialog.milestoneId && milestonesModel)
            {
                milestonesModel.rescheduleMilestone(rescheduleMilestoneDialog.milestoneId, rescheduleMilestoneDialog.currentDate)
                updateMilestones()
            }
            rescheduleMilestoneDialog.close()
        }
    }

    Dialog
    {
        id: infoMilestoneDialog
        title: "Информация о вехе"
        width: 450
        height: 450
        modal: true
        standardButtons: Dialog.Ok
        anchors.centerIn: Overlay.overlay

        property string milestoneId: ""
        property string milestoneName: ""
        property string milestoneAbbr: ""
        property int milestoneStatus: 0
        property date milestoneDate: new Date()
        property date milestoneActualDate: new Date()
        property var rescheduleHistory: []

        Column
        {
            anchors.fill: parent
            anchors.margins: 15
            spacing: 12

            Label { text: "Название: " + infoMilestoneDialog.milestoneName; font.pixelSize: 13 }
            Label { text: "Аббревиатура: " + infoMilestoneDialog.milestoneAbbr; font.pixelSize: 13 }

            Label
            {
                text:
                {
                    var st = infoMilestoneDialog.milestoneStatus
                    if (st === 0) return "Статус: Запланирована"
                    if (st === 1) return "Статус: Пройдена"
                    if (st === 2) return "Статус: Перенесена"
                    return "Статус: Неизвестно"
                }
                font.pixelSize: 13
            }

            Label { text: "Плановая дата: " + Qt.formatDateTime(infoMilestoneDialog.milestoneDate, "dd.MM.yyyy"); font.pixelSize: 13 }

            Label
            {
                visible: infoMilestoneDialog.milestoneStatus === 1
                text: "Дата прохождения: " + Qt.formatDateTime(infoMilestoneDialog.milestoneActualDate, "dd.MM.yyyy")
                font.pixelSize: 13
            }

            Label
            {
                visible: infoMilestoneDialog.rescheduleHistory.length > 0
                text: "История переносов:"
                font.bold: true
                font.pixelSize: 13
            }

            ListView
            {
                visible: infoMilestoneDialog.rescheduleHistory.length > 0
                width: parent.width
                height: 150
                clip: true
                model: infoMilestoneDialog.rescheduleHistory
                delegate: Label
                {
                    text: modelData
                    font.pixelSize: 12
                }
            }
        }
    }

    Menu
    {
        id: milestoneContextMenu
        property string milestoneId: ""
        property date currentDate: new Date()
        property int currentStatus: 0
        property string currentAbbreviation: ""
        property string currentFullName: ""
        property date currentActualDate: new Date()

        MenuItem
        {
            text: "Пройти веху"
            enabled: milestoneContextMenu.currentStatus !== 1
            onTriggered:
            {
                confirmMilestoneDialog.milestoneId = milestoneContextMenu.milestoneId
                confirmMilestoneDialog.milestoneName = milestoneContextMenu.currentAbbreviation
                confirmMilestoneDialog.open()
            }
        }

        MenuItem
        {
            text: "Перенести веху"
            enabled: milestoneContextMenu.currentStatus !== 1
            onTriggered:
            {
                if (milestoneContextMenu.currentStatus === 1) {
                    projectController.notifyMilestoneCompleted()
                    return
                }
                rescheduleMilestoneDialog.milestoneId = milestoneContextMenu.milestoneId
                rescheduleMilestoneDialog.currentDate = milestoneContextMenu.currentDate
                rescheduleMilestoneDialog.open()
            }
        }

        MenuSeparator {}

        MenuItem
        {
            text: "Подробнее"
            onTriggered:
            {
                infoMilestoneDialog.milestoneId = milestoneContextMenu.milestoneId
                infoMilestoneDialog.milestoneName = milestoneContextMenu.currentFullName
                infoMilestoneDialog.milestoneAbbr = milestoneContextMenu.currentAbbreviation
                infoMilestoneDialog.milestoneStatus = milestoneContextMenu.currentStatus
                infoMilestoneDialog.milestoneDate = milestoneContextMenu.currentDate
                infoMilestoneDialog.milestoneActualDate = milestoneContextMenu.currentActualDate
                infoMilestoneDialog.rescheduleHistory = milestonesModel.getMilestone(milestoneContextMenu.milestoneId).rescheduleHistory || []
                infoMilestoneDialog.open()
            }
        }
    }
}
