import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0

Dialog
{
    id: root
    title: "Редактирование сроков задачи"
    width: 560
    height: 280
    modal: true
    anchors.centerIn: Overlay.overlay
    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    property string taskId: ""
    property date startDate: new Date()
    property date endDate: new Date()

    function loadTaskData()
    {
        if (!taskId || taskId === "")
        {
            return false
        }
        var task = projectController?.projectData?.taskModel?.getTask(taskId)
        if (task)
        {
            var s = task.startDate
            var e = task.endDate
            var sValid = s && !isNaN(new Date(s).getTime())
            var eValid = e && !isNaN(new Date(e).getTime())

            if (!sValid)
            {
                s = new Date()
                s.setHours(0, 0, 0, 0)
            }
            if (!eValid)
            {
                e = new Date(s)
                e.setDate(e.getDate() + 5)
                e.setHours(0, 0, 0, 0)
            }

            startDate = s
            endDate = e
            errorLabel.visible = false
            errorLabel.text = "Дата завершения не может быть раньше даты начала"
            startDateField.text = Qt.formatDateTime(startDate, "dd.MM.yyyy")
            endDateField.text = Qt.formatDateTime(endDate, "dd.MM.yyyy")
            return true
        }
        return false
    }

    ColumnLayout
    {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        anchors.topMargin: 4
        anchors.bottomMargin: 8
        spacing: 8

        Label
        {
            text: "Измените даты задачи:"
            font.bold: true
            font.pixelSize: 14
            Layout.fillWidth: true
            leftPadding: 0
        }

        RowLayout
        {
            Layout.fillWidth: true
            spacing: 20

            ColumnLayout
            {
                Layout.fillWidth: true
                spacing: 4

                Label
                {
                    text: "Дата начала:"
                    font.bold: true
                    font.pixelSize: 12
                    leftPadding: 0
                }

                TextField
                {
                    id: startDateField
                    Layout.fillWidth: true
                    Layout.preferredHeight: 36
                    text: Qt.formatDateTime(root.startDate, "dd.MM.yyyy")
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                    leftPadding: 6
                    onEditingFinished:
                    {
                        var parts = text.split(".")
                        if (parts.length === 3)
                        {
                            var d = new Date(parseInt(parts[2]), parseInt(parts[1]) - 1, parseInt(parts[0]))
                            if (!isNaN(d.getTime()))
                            {
                                root.startDate = d
                                text = Qt.formatDateTime(d, "dd.MM.yyyy")
                            }
                            else
                            {
                                text = Qt.formatDateTime(root.startDate, "dd.MM.yyyy")
                            }
                        }
                        else
                        {
                            text = Qt.formatDateTime(root.startDate, "dd.MM.yyyy")
                        }
                    }
                }

                Button
                {
                    text: "Выбрать дату"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    font.pixelSize: 12
                    onClicked: startCalendarPopup.open()
                }

                CalendarPopup
                {
                    id: startCalendarPopup
                    selectedDate: root.startDate
                    onDateSelected: function(d)
                    {
                        root.startDate = d
                        startDateField.text = Qt.formatDateTime(root.startDate, "dd.MM.yyyy")
                    }
                }
            }

            ColumnLayout
            {
                Layout.fillWidth: true
                spacing: 4

                Label
                {
                    text: "Дата завершения:"
                    font.bold: true
                    font.pixelSize: 12
                    leftPadding: 0
                }

                TextField
                {
                    id: endDateField
                    Layout.fillWidth: true
                    Layout.preferredHeight: 36
                    text: Qt.formatDateTime(root.endDate, "dd.MM.yyyy")
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                    leftPadding: 6
                    onEditingFinished:
                    {
                        var parts = text.split(".")
                        if (parts.length === 3)
                        {
                            var d = new Date(parseInt(parts[2]), parseInt(parts[1]) - 1, parseInt(parts[0]))
                            if (!isNaN(d.getTime()))
                            {
                                root.endDate = d
                                text = Qt.formatDateTime(d, "dd.MM.yyyy")
                            }
                            else
                            {
                                text = Qt.formatDateTime(root.endDate, "dd.MM.yyyy")
                            }
                        }
                        else
                        {
                            text = Qt.formatDateTime(root.endDate, "dd.MM.yyyy")
                        }
                    }
                }

                Button
                {
                    text: "Выбрать дату"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 30
                    font.pixelSize: 12
                    onClicked: endCalendarPopup.open()
                }

                CalendarPopup
                {
                    id: endCalendarPopup
                    selectedDate: root.endDate
                    onDateSelected: function(d)
                    {
                        root.endDate = d
                        endDateField.text = Qt.formatDateTime(root.endDate, "dd.MM.yyyy")
                    }
                }
            }
        }

        Label
        {
            id: errorLabel
            color: "red"
            visible: false
            text: "Дата завершения не может быть раньше даты начала"
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            font.pixelSize: 12
            leftPadding: 0
        }

        RowLayout
        {
            Layout.alignment: Qt.AlignRight
            spacing: 8

            Button
            {
                text: "ОК"
                Layout.preferredHeight: 30
                Layout.preferredWidth: 85
                font.pixelSize: 13
                onClicked:
                {
                    errorLabel.text = "Дата завершения не может быть раньше даты начала"

                    if (startDate <= endDate)
                    {
                        if (projectController && projectController.projectData
                            && projectController.projectData.graphKind === 1)
                        {
                            var predEnd = projectController.projectData.getLinkedPredecessorEndDate()
                            if (predEnd && !isNaN(new Date(predEnd).getTime()))
                            {
                                var minStart = new Date(predEnd)
                                minStart.setDate(minStart.getDate() + 1)
                                if (new Date(startDate) < minStart)
                                {
                                    errorLabel.text = "Дата начала не может быть раньше даты завершения предшественника ("
                                                      + Qt.formatDateTime(new Date(predEnd), "dd.MM.yyyy") + ")"
                                    errorLabel.visible = true
                                    return
                                }
                            }
                        }

                        projectController.taskController.updateTaskDates(taskId, startDate, endDate)
                        root.close()
                    }
                    else
                    {
                        errorLabel.visible = true
                    }
                }
            }

            Button
            {
                text: "Отмена"
                Layout.preferredHeight: 30
                Layout.preferredWidth: 85
                font.pixelSize: 13
                onClicked: root.close()
            }
        }
    }

    onOpened:
    {
        loadTaskData()
    }

    function openForTask(tId)
    {
        if (!tId || tId === "")
        {
            return
        }
        root.taskId = tId
        root.open()
    }
}
