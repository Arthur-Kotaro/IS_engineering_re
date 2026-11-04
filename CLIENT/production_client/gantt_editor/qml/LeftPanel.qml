import QtQuick 6.0
import QtQuick.Controls 6.0

Rectangle
{
    id: root
    color: "#fafafa"
    border.color: "#cccccc"
    border.width: 1

    property int rowHeight: 40
    property var flickableRight: null

    readonly property bool isLocalMode:
    {
        return projectController && projectController.projectData && projectController.projectData.graphKind === 1
    }

    readonly property real gridContentHeight:
    {
        if (typeof mainWindow !== "undefined" && mainWindow
            && mainWindow.hasGridArea && mainWindow.hasGridArea())
            return mainWindow.gridArea.contentHeight
            return 0
    }

    readonly property int topBlockHeight: rowHeight * (isLocalMode ? 4 : 5)

    Column
    {
        anchors.fill: parent
        spacing: 0

        Row
        {
            width: parent.width
            height: topBlockHeight

            Rectangle
            {
                width: parent.width - 100
                height: parent.height
                color: "#f0f0f0"
                border.color: "#cccccc"
                border.width: 1

                Column
                {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 5

                    Text
                    {
                        text: "Название проекта:"
                        font.bold: true
                        font.pixelSize: 14
                        color: "#555555"
                    }
                    Text
                    {
                        text: (projectController && projectController.projectData) ? projectController.projectData.projectName : ""
                        font.pixelSize: 22
                        font.bold: true
                        wrapMode: Text.WordWrap
                        width: parent.width
                        color: "#222222"
                    }
                }
            }

            Rectangle
            {
                width: 100
                height: parent.height
                color: "#e8e8e8"
                border.color: "#cccccc"
                border.width: 1

                Column
                {
                    anchors.fill: parent

                    Rectangle
                    {
                        width: parent.width
                        height: rowHeight
                        border.color: "#cccccc"
                        border.width: 1
                        Text { text: "Год"; anchors.centerIn: parent; font.bold: true; font.pixelSize: 12 }
                    }
                    Rectangle
                    {
                        width: parent.width
                        height: rowHeight
                        border.color: "#cccccc"
                        border.width: 1
                        Text { text: "Месяц"; anchors.centerIn: parent; font.bold: true; font.pixelSize: 10 }
                    }
                    Rectangle
                    {
                        width: parent.width
                        height: rowHeight
                        border.color: "#cccccc"
                        border.width: 1
                        Text { text: "Неделя"; anchors.centerIn: parent; font.bold: true; font.pixelSize: 10 }
                    }
                    Rectangle
                    {
                        width: parent.width
                        height: rowHeight
                        border.color: "#cccccc"
                        border.width: 1
                        Text { text: "День"; anchors.centerIn: parent; font.bold: true; font.pixelSize: 12 }
                    }
                    // Строка вех — только в МГ, в ЛГ не занимает места
                    Rectangle
                    {
                        width: parent.width
                        height: root.isLocalMode ? 0 : rowHeight
                        border.color: "#cccccc"
                        border.width: 1
                        visible: !root.isLocalMode
                        Text { text: "Веха"; anchors.centerIn: parent; font.bold: true; font.pixelSize: 12 }
                    }
                }
            }
        }

        Rectangle
        {
            width: parent.width
            height: rowHeight
            color: "#e8e8e8"
            border.color: "#cccccc"
            border.width: 1

            Row
            {
                anchors.fill: parent

                Rectangle
                {
                    width: parent.width - 265
                    height: parent.height
                    border.color: "#cccccc"
                    border.width: 1

                    Row
                    {
                        anchors.fill: parent
                        Rectangle
                        {
                            width: parent.width * 0.64
                            height: parent.height
                            border.color: "#cccccc"
                            border.width: 1
                            Text { text: "Название"; anchors.centerIn: parent; font.bold: true; font.pixelSize: 11 }
                        }
                        Rectangle
                        {
                            width: parent.width * 0.36
                            height: parent.height
                            border.color: "#cccccc"
                            border.width: 1
                            Text { text: "Ответственный"; anchors.centerIn: parent; font.bold: true; font.pixelSize: 10 }
                        }
                    }
                }

                Rectangle
                {
                    width: 65
                    height: parent.height
                    border.color: "#cccccc"
                    border.width: 1
                    Text { text: "Вид"; anchors.centerIn: parent; font.bold: true; font.pixelSize: 11 }
                }

                Rectangle
                {
                    width: 100
                    height: parent.height
                    border.color: "#cccccc"
                    border.width: 1
                    Text { text: "Дата начала"; anchors.centerIn: parent; font.bold: true; font.pixelSize: 10 }
                }

                Rectangle
                {
                    width: 100
                    height: parent.height
                    border.color: "#cccccc"
                    border.width: 1
                    Text { text: "Дата завершения"; anchors.centerIn: parent; font.bold: true; font.pixelSize: 10 }
                }
            }
        }

        // Область задач/групп + жирная нижняя линия
        Item
        {
            id: tasksArea
            width: parent.width
            height: parent.height - 240

            LocalTaskList
            {
                visible: root.isLocalMode
                width: parent.width
                height: parent.height
                flickableRight: root.flickableRight
            }

            Flickable
            {
                id: groupsFlickable
                visible: !root.isLocalMode
                width: parent.width
                height: parent.height
                clip: true
                contentWidth: width
                contentHeight: groupsColumn.height
                boundsBehavior: Flickable.StopAtBounds
                interactive: false
                contentY: root.flickableRight ? root.flickableRight.contentY : 0

                Column
                {
                    id: groupsColumn
                    width: parent.width
                    spacing: 0

                    Repeater
                    {
                        model: (projectController && projectController.projectData) ? projectController.projectData.groupModel : null

                        delegate: GroupDelegate
                        {
                            width: groupsColumn.width
                            flickableRight: root.flickableRight
                        }
                    }
                }
            }

            // Жирная нижняя линия, синхронизированная с гридом справа
            Rectangle
            {
                id: bottomLine
                width: parent.width
                height: 3
                color: "#666666"
                z: 100

                visible: root.gridContentHeight > 0 && y >= 0 && y <= tasksArea.height

                y:
                {
                    if (!root.flickableRight) return root.gridContentHeight
                    return root.gridContentHeight - root.flickableRight.contentY
                }
            }
        }
    }
}
