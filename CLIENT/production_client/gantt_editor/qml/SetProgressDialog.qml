import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0

Dialog
{
    id: root
    title: "Установить прогресс"
    width: 500
    height: 260
    modal: true
    standardButtons: Dialog.NoButton
    anchors.centerIn: Overlay.overlay

    property string taskId: ""

    function openForTask(tId)
    {
        taskId = tId
        var task = projectController?.projectData?.taskModel?.getTask(tId)
        if (task)
        {
            var cur = task.progressCurrent !== undefined ? task.progressCurrent : -1
            var tot = task.progressTotal !== undefined ? task.progressTotal : -1
            currentField.text = (cur >= 0) ? cur.toString() : "0"
            totalField.text = (tot > 0) ? tot.toString() : ""
        }
        else
        {
            currentField.text = "0"
            totalField.text = ""
        }
        errorLabel.visible = false
        open()
    }

    function validate()
    {
        var c = parseInt(currentField.text)
        var t = parseInt(totalField.text)
        if (isNaN(c) || isNaN(t)) return "Введите целые числа"
        if (t <= 0) return "Общее количество должно быть больше нуля"
        if (c < 0) return "Текущее значение не может быть отрицательным"
        if (c > t) return "Текущее значение не может превышать общее"
        return ""
    }

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 12

        Label
        {
            text: "Введите прогресс задачи:"
            font.pixelSize: 14
            Layout.fillWidth: true
        }

        RowLayout
        {
            Layout.fillWidth: true
            spacing: 10

            Label
            {
                text: "Выполнено:"
                font.pixelSize: 13
                Layout.preferredWidth: 100
            }

            TextField
            {
                id: currentField
                Layout.fillWidth: true
                Layout.preferredHeight: 34
                font.pixelSize: 13
                horizontalAlignment: Text.AlignHCenter
                inputMethodHints: Qt.ImhDigitsOnly
            }
        }

        RowLayout
        {
            Layout.fillWidth: true
            spacing: 10

            Label
            {
                text: "Всего:"
                font.pixelSize: 13
                Layout.preferredWidth: 100
            }

            TextField
            {
                id: totalField
                Layout.fillWidth: true
                Layout.preferredHeight: 34
                font.pixelSize: 13
                horizontalAlignment: Text.AlignHCenter
                inputMethodHints: Qt.ImhDigitsOnly
            }
        }

        Label
        {
            id: errorLabel
            color: "#d32f2f"
            visible: false
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            font.pixelSize: 12
        }

        Item { Layout.fillHeight: true }

        RowLayout
        {
            Layout.fillWidth: true
            spacing: 10

            Item { Layout.fillWidth: true }

            Button
            {
                text: "ОК"
                Layout.preferredWidth: 120
                Layout.preferredHeight: 34
                onClicked:
                {
                    var err = root.validate()
                    if (err !== "")
                    {
                        errorLabel.text = err
                        errorLabel.visible = true
                        return
                    }
                    var c = parseInt(currentField.text)
                    var t = parseInt(totalField.text)
                    if (projectController && root.taskId)
                        projectController.taskController.setTaskProgress(root.taskId, c, t)
                    root.close()
                }
            }

            Button
            {
                text: "Отмена"
                Layout.preferredWidth: 120
                Layout.preferredHeight: 34
                onClicked: root.close()
            }
        }
    }
}
