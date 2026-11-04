import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Window 6.0
import GanttEditor 1.0

ApplicationWindow
{
    id: mainWindow
    width: 1800
    height: 750
    minimumWidth: 1800
    minimumHeight: 750
    visible: true
    title: "Мастерграфик: " + (projectController.projectData ? projectController.projectData.projectName : "")

    property int projectId: argProjectId
    property string mgId: argMgId
    property string accessToken: argToken

    property bool inEditMode: (projectController && projectController.inEditMode) || false
    property int leftPanelWidth: 525

    readonly property bool editingText:
    {
        var item = activeFocusItem
        if (!item) return false
        return (item instanceof TextInput || item instanceof TextField || item instanceof TextArea || item instanceof TextEdit)
    }

    function hasGridArea()
    {
        return typeof mainWindow.gridArea !== "undefined" && mainWindow.gridArea !== null
    }

    property alias gridArea: gridArea
    property alias leftPanel: leftPanel
    property alias calendarHeader: calendarHeader

    property alias cascadeDialog: cascadeDialog
    property alias listDialog: listDialog
    property alias newTaskDialog: newTaskDialog
    property alias editTaskDialog: editTaskDialog
    property alias editForecastDatesDialog: editForecastDatesDialog
    property alias responsibleDialog: responsibleDialog
    property alias commentDialog: commentDialog
    property alias setProgressDialog: setProgressDialog
    property alias completeTaskDialog: completeTaskDialog
    property alias dependencyDialog: dependencyDialog
    property alias detachLocalGraphDialog: detachLocalGraphDialog
    property alias localGraphExistsDialog: localGraphExistsDialog
    property alias confirmRemoveTaskDialog: confirmRemoveTaskDialog
    property alias renameTaskDialog: renameTaskDialog
    property alias renameGroupDialog: renameGroupDialog
    property alias settingsDialog: settingsDialog
    property alias aboutDialog: aboutDialog
    property alias helpDialog: helpDialog
    property alias exportDialog: exportDialog
    property alias taskContextMenu: taskContextMenu

    Component.onCompleted: {
        if (mgId !== "")
            projectController.loadFromServer(mgId, accessToken)
    }

    Connections
    {
        target: projectController
        function onProjectLoaded()
        {
            if (mainWindow.hasGridArea()) mainWindow.gridArea.updateData()
            if (calendarHeader) calendarHeader.refresh()
        }
        function onProjectDataChanged()
        {
            if (mainWindow.hasGridArea()) mainWindow.gridArea.updateData()
            if (calendarHeader) calendarHeader.refresh()
        }
        function onErrorOccurred(message)
        {
            if (typeof toast !== "undefined" && toast) toast.show(message)
        }
    }

    AppToolBar { id: appToolBar; anchors.top: parent.top; anchors.left: parent.left; anchors.right: parent.right }

    Item
    {
        id: contentArea
        anchors.top: appToolBar.bottom
        anchors.bottom: infoPanel.top
        anchors.left: parent.left
        anchors.right: parent.right

        Row
        {
            visible: inEditMode
            anchors.fill: parent
            spacing: 0

            LeftPanel
            {
                flickableRight: flickableRight
                id: leftPanel
                width: leftPanelWidth
                height: parent.height
            }

            Rectangle
            {
                id: splitter
                width: 6
                height: parent.height
                color: "#888888"

                MouseArea
                {
                    anchors.fill: parent
                    anchors.leftMargin: -4
                    anchors.rightMargin: -4
                    cursorShape: Qt.SizeHorCursor
                    drag.target: splitter
                    drag.axis: Drag.XAxis
                    drag.minimumX: 525
                    drag.maximumX: 800

                    onPositionChanged:
                    {
                        if (drag.active) leftPanelWidth = splitter.x
                    }
                }
            }

            Rectangle
            {
                width: parent.width - leftPanelWidth - 6
                height: parent.height
                color: "white"
                clip: true

                CalendarHeader
                {
                    id: calendarHeader
                    x: -flickableRight.contentX
                    width: calendarHeader.contentWidth
                }

                Flickable
                {
                    id: flickableRight
                    anchors.top: calendarHeader.bottom
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    contentWidth: gridArea.width
                    contentHeight: gridArea.height
                    boundsBehavior: Flickable.StopAtBounds
                    clip: true
                    ScrollBar.horizontal: ScrollBar { policy: ScrollBar.AlwaysOn }
                    ScrollBar.vertical: ScrollBar { policy: ScrollBar.AlwaysOn }

                    GridArea
                    {
                        id: gridArea
                        externalFlickable: flickableRight
                        width: calendarHeader.contentWidth
                        taskContextMenu: taskContextMenu
                    }
                }
            }
        }
    }

    InfoPanel { id: infoPanel; anchors.bottom: parent.bottom; anchors.left: parent.left; anchors.right: parent.right }

    // ---------- Глобальный MouseArea (Ctrl+Scroll, Shift+Scroll) ----------
    MouseArea
    {
        anchors.fill: parent
        acceptedButtons: Qt.NoButton
        z: 100
        onWheel: function(wheel)
        {
            var flick = flickableRight
            if (!flick) return

            if (wheel.modifiers & Qt.ControlModifier)
            {
                if (wheel.angleDelta.y > 0) projectController.settingsManager.setZoomLevel(0)
                else projectController.settingsManager.setZoomLevel(1)
                wheel.accepted = true
                return
            }

            if (wheel.modifiers & Qt.ShiftModifier)
            {
                flick.contentX = Math.max(0, Math.min(flick.contentX - wheel.angleDelta.y, flick.contentWidth - flick.width))
            }
            else
            {
                flick.contentY = Math.max(0, Math.min(flick.contentY - wheel.angleDelta.y, flick.contentHeight - flick.height))
            }
            wheel.accepted = true
        }
    }

    // ---------- Контекстное меню задач ----------
    TaskContextMenu { id: taskContextMenu }

    // ---------- Диалоги ----------
    CascadeDialog { id: cascadeDialog }
    ListDialog { id: listDialog }
    NewTaskDialog { id: newTaskDialog }
    EditTaskDialog { id: editTaskDialog }
    EditForecastDatesDialog { id: editForecastDatesDialog }
    ResponsibleDialog { id: responsibleDialog }
    CommentDialog { id: commentDialog }
    SetProgressDialog { id: setProgressDialog }
    CompleteTaskDialog { id: completeTaskDialog; toastRef: toast }
    DependencyDialog { id: dependencyDialog }
    DetachLocalGraphDialog { id: detachLocalGraphDialog }
    LocalGraphExistsDialog { id: localGraphExistsDialog }
    ConfirmRemoveTaskDialog { id: confirmRemoveTaskDialog }
    RenameTaskDialog { id: renameTaskDialog }
    RenameGroupDialog { id: renameGroupDialog }
    SettingsDialog { id: settingsDialog }
    AboutDialog { id: aboutDialog }
    HelpDialog { id: helpDialog }
    ExportDialog { id: exportDialog }

    // ---------- Toast (уведомления) ----------
    Rectangle
    {
        id: toast
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 60
        width: Math.min(parent.width - 40, toastText.implicitWidth + 40)
        height: toastText.implicitHeight + 24
        color: "#d32f2f"
        radius: 6
        opacity: 0
        visible: opacity > 0
        z: 200

        Text
        {
            id: toastText
            anchors.centerIn: parent
            color: "white"
            font.pixelSize: 14
            wrapMode: Text.WordWrap
            width: parent.width - 32
            horizontalAlignment: Text.AlignHCenter
        }

        Timer
        {
            id: toastTimer
            interval: 3000
            onTriggered: toastHide.start()
        }

        NumberAnimation { id: toastShow; target: toast; property: "opacity"; to: 1.0; duration: 200 }
        NumberAnimation { id: toastHide; target: toast; property: "opacity"; to: 0.0; duration: 300 }

        function show(message)
        {
            toastText.text = message
            toastShow.start()
            toastTimer.restart()
        }
    }
}
