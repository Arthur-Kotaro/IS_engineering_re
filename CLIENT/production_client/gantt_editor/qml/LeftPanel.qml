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

    readonly property bool isPure: projectController ? projectController.isPure : false
    readonly property bool canEdit: projectController ? projectController.canEdit : false

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
                        text: root.isLocalMode ? "Название задачи:" : "Название проекта:"
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

                    Rectangle { width: parent.width; height: rowHeight; border.color: "#cccccc"; border.width: 1; Text { text: "Год"; anchors.centerIn: parent; font.bold: true; font.pixelSize: 12 } }
                    Rectangle { width: parent.width; height: rowHeight; border.color: "#cccccc"; border.width: 1; Text { text: "Месяц"; anchors.centerIn: parent; font.bold: true; font.pixelSize: 10 } }
                    Rectangle { width: parent.width; height: rowHeight; border.color: "#cccccc"; border.width: 1; Text { text: "Неделя"; anchors.centerIn: parent; font.bold: true; font.pixelSize: 10 } }
                    Rectangle { width: parent.width; height: rowHeight; border.color: "#cccccc"; border.width: 1; Text { text: "День"; anchors.centerIn: parent; font.bold: true; font.pixelSize: 12 } }
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
                        Rectangle { width: parent.width * 0.64; height: parent.height; border.color: "#cccccc"; border.width: 1; Text { text: "Название"; anchors.centerIn: parent; font.bold: true; font.pixelSize: 11 } }
                        Rectangle { width: parent.width * 0.36; height: parent.height; border.color: "#cccccc"; border.width: 1; Text { text: "Ответственный"; anchors.centerIn: parent; font.bold: true; font.pixelSize: 10 } }
                    }
                }

                Rectangle { width: 65; height: parent.height; border.color: "#cccccc"; border.width: 1; Text { text: "Вид"; anchors.centerIn: parent; font.bold: true; font.pixelSize: 11 } }
                Rectangle { width: 100; height: parent.height; border.color: "#cccccc"; border.width: 1; Text { text: "Дата начала"; anchors.centerIn: parent; font.bold: true; font.pixelSize: 10 } }
                Rectangle { width: 100; height: parent.height; border.color: "#cccccc"; border.width: 1; Text { text: "Дата завершения"; anchors.centerIn: parent; font.bold: true; font.pixelSize: 10 } }
            }
        }

        Item
        {
            id: tasksArea
            width: parent.width
            height: parent.height - 240

            // Pure: пустые строки
            Column
            {
                visible: root.isPure
                width: parent.width
                Repeater
                {
                    model: 5
                    Rectangle
                    {
                        width: parent.width
                        height: root.rowHeight
                        color: "transparent"
                        border.color: "#eeeeee"
                        border.width: 1
                    }
                }
            }

            LocalTaskList
            {
                visible: root.isLocalMode && !root.isPure
                width: parent.width
                height: parent.height
                flickableRight: root.flickableRight
            }

            Flickable
            {
                id: groupsFlickable
                visible: !root.isLocalMode && !root.isPure
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

            // MouseArea для контекстного меню на пустом месте
            MouseArea
            {
                anchors.fill: parent
                visible: root.isPure && root.canEdit
                acceptedButtons: Qt.RightButton
                z: 50
                onClicked: function(mouse) {
                    if (mouse.button !== Qt.RightButton) return
                    leftPanelMenu.popup()
                }
            }

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

    Menu
    {
        id: leftPanelMenu

        MenuItem
        {
            text: "Создать группы по умолчанию"
            visible: root.isPure && !root.isLocalMode
            onTriggered: {
                if (mainWindow && mainWindow.defaultGroupsDialog)
                    mainWindow.defaultGroupsDialog.open()
            }
        }

        MenuItem
        {
            text: "Создать группу"
            visible: !root.isLocalMode
            onTriggered: {
                if (mainWindow && mainWindow.createGroupDialog)
                    mainWindow.createGroupDialog.open()
            }
        }

        MenuItem
        {
            text: "Создать задачу"
            visible: root.canEdit
            onTriggered: {
                if (mainWindow && mainWindow.newTaskDialog && projectController) {
                    var gm = projectController.projectData.groupModel
                    var gid = gm.rowCount() > 0 ? gm.getGroupId(0) : "default"
                    mainWindow.newTaskDialog.openForGroup(gid)
                }
            }
        }
    }
}
