import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0

Dialog
{
    id: root
    title: "Редактирование прогнозных сроков задачи"
    width: 480
    height: 260
    modal: true
    anchors.centerIn: Overlay.overlay

    property string taskId: ""
    property date forecastStart: new Date()
    property date forecastEnd: new Date()

    function loadTaskData()
    {
        if (!taskId || taskId === "") return false
        var task = projectController?.projectData?.taskModel?.getTask(taskId)
        if (task)
        {
            var fs = task.forecastStart
            var fe = task.forecastEnd
            var fsValid = fs && !isNaN(new Date(fs).getTime())
            var feValid = fe && !isNaN(new Date(fe).getTime())

            if (!fsValid)
            {
                fs = new Date()
                fs.setHours(0, 0, 0, 0)
            }
            if (!feValid)
            {
                fe = new Date(fs)
                fe.setDate(fe.getDate() + 5)
                fe.setHours(0, 0, 0, 0)
            }

            forecastStart = fs
            forecastEnd = fe
            errorLabel.visible = false
            startField.text = Qt.formatDateTime(forecastStart, "dd.MM.yyyy")
            endField.text = Qt.formatDateTime(forecastEnd, "dd.MM.yyyy")
            return true
        }
        return false
    }

    function openForTask(tId)
    {
        if (!tId || tId === "") return
        var task = projectController?.projectData?.taskModel?.getTask(tId)
        if (task && task.status === 1)
        {
            return
        }
        root.taskId = tId
        root.open()
    }

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 12

        Label
        {
            text: "Измените прогнозные сроки задачи:"
            font.bold: true
            font.pixelSize: 14
            Layout.fillWidth: true
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
                    text: "Прогноз начала:"
                    font.bold: true
                    font.pixelSize: 12
                }

                TextField
                {
                    id: startField
                    Layout.fillWidth: true
                    Layout.preferredHeight: 34
                    font.pixelSize: 13
                    horizontalAlignment: Text.AlignHCenter
                    onEditingFinished:
                    {
                        var parts = text.split(".")
                        if (parts.length === 3)
                        {
                            var d = new Date(parseInt(parts[2]), parseInt(parts[1]) - 1, parseInt(parts[0]))
                            if (!isNaN(d.getTime()))
                            {
                                root.forecastStart = d
                                text = Qt.formatDateTime(d, "dd.MM.yyyy")
                            }
                            else
                            {
                                text = Qt.formatDateTime(root.forecastStart, "dd.MM.yyyy")
                            }
                        }
                        else
                        {
                            text = Qt.formatDateTime(root.forecastStart, "dd.MM.yyyy")
                        }
                    }
                }
            }

            ColumnLayout
            {
                Layout.fillWidth: true
                spacing: 4

                Label
                {
                    text: "Прогноз завершения:"
                    font.bold: true
                    font.pixelSize: 12
                }

                TextField
                {
                    id: endField
                    Layout.fillWidth: true
                    Layout.preferredHeight: 34
                    font.pixelSize: 13
                    horizontalAlignment: Text.AlignHCenter
                    onEditingFinished:
                    {
                        var parts = text.split(".")
                        if (parts.length === 3)
                        {
                            var d = new Date(parseInt(parts[2]), parseInt(parts[1]) - 1, parseInt(parts[0]))
                            if (!isNaN(d.getTime()))
                            {
                                root.forecastEnd = d
                                text = Qt.formatDateTime(d, "dd.MM.yyyy")
                            }
                            else
                            {
                                text = Qt.formatDateTime(root.forecastEnd, "dd.MM.yyyy")
                            }
                        }
                        else
                        {
                            text = Qt.formatDateTime(root.forecastEnd, "dd.MM.yyyy")
                        }
                    }
                }
            }
        }

        Label
        {
            id: errorLabel
            color: "red"
            visible: false
            text: "Дата завершения прогноза не может быть раньше даты начала"
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            font.pixelSize: 12
        }

        Item { Layout.fillHeight: true }

        RowLayout
        {
            Layout.alignment: Qt.AlignRight
            spacing: 8

            Button
            {
                text: "ОК"
                Layout.preferredHeight: 32
                Layout.preferredWidth: 90
                font.pixelSize: 13
                onClicked:
                {
                    if (root.forecastStart <= root.forecastEnd)
                    {
                        if (projectController)
                            projectController.taskController.updateForecastDates(root.taskId, root.forecastStart, root.forecastEnd)
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
                Layout.preferredHeight: 32
                Layout.preferredWidth: 90
                font.pixelSize: 13
                onClicked: root.close()
            }
        }
    }

    onOpened: loadTaskData()
}
