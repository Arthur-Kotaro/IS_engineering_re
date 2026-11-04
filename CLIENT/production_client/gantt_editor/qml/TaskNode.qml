import QtQuick 6.0
import QtQuick.Controls 6.0

Rectangle
{
    id: root
    property var rowData
    property var taskContextMenu

    signal tooltipRequested(bool visible, var mouseArea, bool fromNode)
    signal tooltipHidden()

    width: 30
    height: 30
    radius: 15
    color:
    {
        if (!rowData) return "#FFD700"
        switch (rowData.taskStatus)
        {
            case 0: return "#FFD700"
            case 1: return "#32CD32"
            case 2: return "#FF8C00"
            case 3: return "#FF4444"
            default: return "#FFD700"
        }
    }
    z: 4

    MouseArea
    {
        id: nodeArea
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        hoverEnabled: true

        property bool showTooltip: false

        Timer
        {
            id: nodeTooltipDelay
            interval: 500
            repeat: false
            onTriggered:
            {
                nodeArea.showTooltip = true
                root.tooltipRequested(true, nodeArea, true)
            }
        }

        onEntered: nodeTooltipDelay.restart()
        onExited:
        {
            nodeTooltipDelay.stop()
            nodeArea.showTooltip = false
            root.tooltipHidden()
        }

        onClicked: function(mouse)
        {
            if (mouse.button === Qt.RightButton && rowData && root.taskContextMenu)
            {
                root.taskContextMenu.taskId = rowData.taskId
                root.taskContextMenu.popup()
            }
        }
    }
}
