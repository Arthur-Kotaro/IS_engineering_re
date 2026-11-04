import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0

Dialog
{
    id: root
    title: "Отвязать локальный график"
    width: 500
    height: 240
    modal: true
    standardButtons: Dialog.NoButton
    anchors.centerIn: Overlay.overlay

    property string taskId: ""

    function openForTask(tId)
    {
        taskId = tId
        open()
    }

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        Label
        {
            text: "Локальный график будет отвязан от задачи."
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            font.pixelSize: 14
        }

        Label
        {
            text: "Удалить файл локального графика с диска или оставить его?"
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            font.pixelSize: 13
            color: "#555555"
        }

        Item { Layout.fillHeight: true }

        RowLayout
        {
            Layout.fillWidth: true
            spacing: 10

            Button
            {
                text: "Удалить файл"
                Layout.fillWidth: true
                Layout.preferredHeight: 36
                onClicked:
                {
                    if (projectController && root.taskId)
                        projectController.localGraphController.detachLocalGraph(root.taskId, true)
                    root.close()
                }
            }

            Button
            {
                text: "Оставить файл"
                Layout.fillWidth: true
                Layout.preferredHeight: 36
                onClicked:
                {
                    if (projectController && root.taskId)
                        projectController.localGraphController.detachLocalGraph(root.taskId, false)
                    root.close()
                }
            }

            Button
            {
                text: "Отмена"
                Layout.fillWidth: true
                Layout.preferredHeight: 36
                onClicked: root.close()
            }
        }
    }
}
