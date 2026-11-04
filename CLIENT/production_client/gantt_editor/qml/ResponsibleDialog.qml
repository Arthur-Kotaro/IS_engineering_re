import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0

Dialog
{
    id: root
    title: "Назначить ответственного"
    width: 400
    height: 230
    modal: true
    standardButtons: Dialog.NoButton
    anchors.centerIn: Overlay.overlay

    property string taskId: ""

    function openForTask(tId)
    {
        taskId = tId
        var task = projectController?.projectData?.taskModel?.getTask(tId)
        responsibleField.text = task ? (task.responsible || "") : ""
        open()
    }

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 15

        Label
        {
            text: "ФИО ответственного:"
            Layout.fillWidth: true
            font.pixelSize: 13
        }

        TextField
        {
            id: responsibleField
            Layout.fillWidth: true
            Layout.preferredHeight: 35
            placeholderText: "Введите ФИО"
            font.pixelSize: 13
            focus: true
            onAccepted:
            {
                if (responsibleField.text !== "")
                    root.accept()
            }
        }

        Item { Layout.fillHeight: true }

        RowLayout
        {
            Layout.fillWidth: true
            spacing: 10

            Button
            {
                text: "Отмена"
                Layout.fillWidth: true
                Layout.preferredHeight: 35
                onClicked: root.close()
            }

            Button
            {
                text: "ОК"
                Layout.fillWidth: true
                Layout.preferredHeight: 35
                enabled: responsibleField.text !== ""
                onClicked: root.accept()
            }
        }
    }

    onAccepted:
    {
        if (taskId && projectController && projectController.projectData)
        {
            var task = projectController.projectData.taskModel.getTask(taskId)
            if (task)
            {
                projectController.projectData.taskModel.updateTask(
                    taskId,
                    task.title,
                    responsibleField.text,
                    task.startDate,
                    task.endDate,
                    task.status
                )
            }
        }
    }
}
