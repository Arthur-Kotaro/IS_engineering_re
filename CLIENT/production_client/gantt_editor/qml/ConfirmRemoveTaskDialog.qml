import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0

Dialog
{
    id: root
    title: "Удаление задачи"
    width: 700
    height: 260
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
            text: "Задача имеет связанный локальный график."
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            font.pixelSize: 14
        }

        Label
        {
            text: "Удалить задачу? Файл локального графика останется на диске."
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            font.pixelSize: 13
            color: "#555555"
        }

        Item { Layout.fillHeight: true }

        RowLayout
        {
            Layout.fillWidth: true
            spacing: 12

            Item { Layout.fillWidth: true }

            Button
            {
                text: "Удалить задачу"
                Layout.preferredWidth: 180
                Layout.preferredHeight: 36
                font.pixelSize: 13
                onClicked:
                {
                    if (projectController && root.taskId)
                        projectController.taskController.confirmRemoveTask(root.taskId)
                    root.close()
                }
            }

            Button
            {
                text: "Отмена"
                Layout.preferredWidth: 120
                Layout.preferredHeight: 36
                font.pixelSize: 13
                onClicked: root.close()
            }
        }
    }
}
