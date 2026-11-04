import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0

Dialog
{
    id: root
    title: "Локальный график уже существует"
    width: 760
    height: 280
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
            text: "Файл локального графика для этой задачи уже существует."
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            font.pixelSize: 14
        }

        Label
        {
            text: "Использовать его как источник прогноза или перезаписать новым пустым файлом?"
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
                text: "Использовать существующий"
                Layout.preferredWidth: 260
                Layout.preferredHeight: 36
                font.pixelSize: 13
                onClicked:
                {
                    if (projectController && root.taskId)
                        projectController.localGraphController.attachExistingLocalGraph(root.taskId)
                    root.close()
                }
            }

            Button
            {
                text: "Перезаписать"
                Layout.preferredWidth: 160
                Layout.preferredHeight: 36
                font.pixelSize: 13
                onClicked:
                {
                    if (projectController && root.taskId)
                        projectController.localGraphController.createLocalGraphOverwrite(root.taskId)
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
