import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Window 6.0

ApplicationWindow
{
    id: exportWindow
    visible: false
    width: 100
    height: 100

    readonly property int leftPanelWidth: 525

    property bool exportShowProgress: true
    property bool exportShowDependencies: true
    property bool exportShowTaskHistory: true
    property bool exportShowComments: true
    property bool exportShowHolidays: false
    property int exportViewMode: 0
    property int exportZoomLevel: 0

    property real fullWidth: leftPanelWidth + 6 + gridArea.width
    property real fullHeight: Math.max(leftPanel.height, calendarHeader.height + gridArea.contentHeight)

    Item
    {
        id: contentArea
        objectName: "contentArea"
        width: exportWindow.fullWidth
        height: exportWindow.fullHeight
        clip: false

        Row
        {
            anchors.fill: parent
            spacing: 0

            LeftPanel
            {
                id: leftPanel
                flickableRight: null
                width: exportWindow.leftPanelWidth
                height: parent.height
            }

            Rectangle
            {
                id: splitter
                width: 6
                height: parent.height
                color: "#888888"
            }

            Rectangle
            {
                width: parent.width - exportWindow.leftPanelWidth - 6
                height: parent.height
                color: "white"
                clip: true

                CalendarHeader
                {
                    id: calendarHeader
                    x: 0
                    width: calendarHeader.contentWidth
                }

                Item
                {
                    anchors.top: calendarHeader.bottom
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    clip: true

                    GridArea
                    {
                        id: gridArea
                        externalFlickable: null
                        width: calendarHeader.contentWidth
                        taskContextMenu: null
                        showProgress: exportWindow.exportShowProgress
                        showDependencies: exportWindow.exportShowDependencies
                        showTaskHistory: exportWindow.exportShowTaskHistory
                        showComments: exportWindow.exportShowComments
                    }
                }
            }
        }
    }

    function applySettings(settings)
    {
        console.log("=== applySettings START ===")
        console.log("  contentArea: " + contentArea.width + " x " + contentArea.height)
        console.log("  gridArea: " + gridArea.width + " x " + gridArea.height)
        console.log("  calendarHeader.contentWidth: " + calendarHeader.contentWidth)
        console.log("  leftPanel: " + leftPanel.width + " x " + leftPanel.height)

        if (!settings) {
            console.log("=== applySettings: no settings ===")
            return
        }

        if (settings.showProgress !== undefined) exportShowProgress = settings.showProgress
        if (settings.showDependencies !== undefined) exportShowDependencies = settings.showDependencies
        if (settings.showTaskHistory !== undefined) exportShowTaskHistory = settings.showTaskHistory
        if (settings.showComments !== undefined) exportShowComments = settings.showComments
        if (settings.showHolidays !== undefined) exportShowHolidays = settings.showHolidays
        if (settings.viewMode !== undefined) exportViewMode = settings.viewMode
        if (settings.zoomLevel !== undefined) exportZoomLevel = settings.zoomLevel

        if (projectController && projectController.settingsManager)
        {
            projectController.settingsManager.setViewMode(exportViewMode)
            projectController.settingsManager.setZoomLevel(exportZoomLevel)
            projectController.settingsManager.showHolidays = exportShowHolidays
        }

        if (gridArea) gridArea.updateData()
        if (calendarHeader) {
            calendarHeader.updateDisplayRange()
            calendarHeader.refresh()
        }

        console.log("=== applySettings AFTER update ===")
        console.log("  contentArea: " + contentArea.width + " x " + contentArea.height)
        console.log("  gridArea: " + gridArea.width + " x " + gridArea.height)
        console.log("  calendarHeader.contentWidth: " + calendarHeader.contentWidth)
        console.log("  leftPanel: " + leftPanel.width + " x " + leftPanel.height)
        console.log("  exportWindow.fullWidth: " + exportWindow.fullWidth)
        console.log("  exportWindow.fullHeight: " + exportWindow.fullHeight)
        console.log("=== applySettings END ===")
    }
}
