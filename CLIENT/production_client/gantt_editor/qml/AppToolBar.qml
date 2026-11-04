import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0

Rectangle
{
    id: appToolBar
    height: 40
    color: "#e0e0e0"
    z: 10

    readonly property bool isLocalMode: {
        return projectController && projectController.projectData
               && projectController.projectData.graphKind === 1
    }

    function collapseAll()
    {
        if (!projectController || !projectController.projectData) return
        var gm = projectController.projectData.groupModel
        var ids = gm.getGroupIds()
        for (var i = 0; i < ids.length; i++)
            gm.setGroupExpanded(ids[i], false)
    }

    function expandAll()
    {
        if (!projectController || !projectController.projectData) return
        var gm = projectController.projectData.groupModel
        var ids = gm.getGroupIds()
        for (var i = 0; i < ids.length; i++)
            gm.setGroupExpanded(ids[i], true)
    }

    RowLayout
    {
        anchors.fill: parent
        anchors.leftMargin: 4
        anchors.rightMargin: 4
        spacing: 3

        // ---- Сохранить, Обновить, Экспорт ----
        ToolButton
        {
            icon.source: "qrc:/qml/icons/save.svg"
            icon.width: 20; icon.height: 20
            ToolTip.visible: hovered; ToolTip.delay: 400
            ToolTip.text: "Сохранить (отправить diff на сервер)"
            onClicked:
            {
                if (projectController)
                    projectController.saveProject()
            }
        }
        ToolButton
        {
            icon.source: "qrc:/qml/icons/refresh.svg"
            icon.width: 18; icon.height: 18
            Layout.preferredWidth: 28
            ToolTip.visible: hovered; ToolTip.delay: 400
            ToolTip.text: "Обновить (snapshot с сервера)"
            onClicked:
            {
                if (projectController && mainWindow.mgId !== "")
                    projectController.loadFromServer(mainWindow.mgId, mainWindow.accessToken)
            }
        }
        ToolButton
        {
            icon.source: "qrc:/qml/icons/export.svg"
            icon.width: 20; icon.height: 20
            ToolTip.visible: hovered; ToolTip.delay: 400
            ToolTip.text: "Экспорт графика"
            onClicked: { if (mainWindow && mainWindow.exportDialog) mainWindow.exportDialog.open() }
        }

        // Разделитель
        Rectangle { width: 1; height: 24; color: "#b0b0b0"; Layout.alignment: Qt.AlignVCenter }

        // ---- Блокировка, Настройки ----
        ToolButton
        {
            icon.source: (projectController && projectController.settingsManager.editingLocked)
                         ? "qrc:/qml/icons/lock.svg" : "qrc:/qml/icons/unlock.svg"
            icon.width: 20; icon.height: 20
            checkable: true
            checked: projectController ? projectController.settingsManager.editingLocked : false
            ToolTip.visible: hovered; ToolTip.delay: 400
            ToolTip.text: checked ? "Редактирование заблокировано" : "Редактирование разрешено"
            onCheckedChanged: { if (projectController) projectController.settingsManager.editingLocked = checked }
        }

        ToolButton
        {
            icon.source: "qrc:/qml/icons/settings.svg"
            icon.width: 20; icon.height: 20
            ToolTip.visible: hovered; ToolTip.delay: 400
            ToolTip.text: "Настройки"
            onClicked: if (mainWindow.settingsDialog) mainWindow.settingsDialog.open()
        }

        Rectangle { width: 1; height: 24; color: "#b0b0b0"; Layout.alignment: Qt.AlignVCenter }

        ToolButton
        {
            icon.source: "qrc:/qml/icons/collapse.svg"
            icon.width: 18; icon.height: 18
            Layout.preferredWidth: 28
            ToolTip.visible: hovered; ToolTip.delay: 400
            ToolTip.text: "Свернуть все группы"
            onClicked: appToolBar.collapseAll()
        }
        ToolButton
        {
            icon.source: "qrc:/qml/icons/expand.svg"
            icon.width: 18; icon.height: 18
            Layout.preferredWidth: 28
            ToolTip.visible: hovered; ToolTip.delay: 400
            ToolTip.text: "Развернуть все группы"
            onClicked: appToolBar.expandAll()
        }

        // Разделитель
        Rectangle { width: 1; height: 24; color: "#b0b0b0"; Layout.alignment: Qt.AlignVCenter }

        // ---- Каскад, Список ----
        Button
        {
            text: "Добавить каскад"
            Layout.preferredWidth: 150
            font.pixelSize: 11
            onClicked: if (mainWindow.cascadeDialog) mainWindow.cascadeDialog.open()
        }
        Button
        {
            text: "Добавить список"
            Layout.preferredWidth: 150
            font.pixelSize: 11
            onClicked: if (mainWindow.listDialog) mainWindow.listDialog.open()
        }

        Rectangle { width: 1; height: 24; color: "#b0b0b0"; Layout.alignment: Qt.AlignVCenter }

        // ---- Режим, Масштаб, Экран ----
        ComboBox
        {
            id: viewModeCombo
            visible: !appToolBar.isLocalMode
            Layout.preferredWidth: 190
            font.pixelSize: 11
            model: ["Целевой", "Прогнозный", "Комбинированный"]
            currentIndex: projectController ? projectController.settingsManager.viewMode : 0
            onActivated: { if (projectController) projectController.settingsManager.setViewMode(currentIndex) }
            Connections
            {
                target: projectController ? projectController.settingsManager : null
                function onViewModeChanged()
                {
                    if (projectController) viewModeCombo.currentIndex = projectController.settingsManager.viewMode
                }
            }
        }
        ComboBox
        {
            id: zoomCombo
            Layout.preferredWidth: 90
            font.pixelSize: 11
            model: ["День", "Неделя"]
            currentIndex: projectController ? projectController.settingsManager.zoomLevel : 0
            onActivated: { if (projectController) projectController.settingsManager.setZoomLevel(currentIndex) }
            Connections
            {
                target: projectController ? projectController.settingsManager : null
                function onZoomLevelChanged()
                {
                    if (projectController) zoomCombo.currentIndex = projectController.settingsManager.zoomLevel
                }
            }
        }
        ComboBox
        {
            id: screenCombo
            Layout.preferredWidth: 110
            font.pixelSize: 11
            model: ["В окне", "Весь экран"]
            currentIndex: mainWindow.visibility === Window.FullScreen ? 1 : 0
            onActivated:
            {
                if (currentIndex === 1) mainWindow.showFullScreen()
                else mainWindow.showNormal()
            }
            Connections
            {
                target: mainWindow
                function onVisibilityChanged()
                {
                    screenCombo.currentIndex = (mainWindow.visibility === Window.FullScreen) ? 1 : 0
                }
            }
        }

        Rectangle { width: 1; height: 24; color: "#b0b0b0"; Layout.alignment: Qt.AlignVCenter }

        // ---- Переключатели ----
        Button
        {
            text: "Прогресс"
            visible: !appToolBar.isLocalMode
            checked: mainWindow.hasGridArea() ? mainWindow.gridArea.showProgress : true
            checkable: true
            Layout.preferredWidth: 95
            font.pixelSize: 11
            onCheckedChanged: { if(mainWindow.hasGridArea()) mainWindow.gridArea.showProgress = checked }
        }
        Button
        {
            text: "Зависимости"
            checked: mainWindow.hasGridArea() ? mainWindow.gridArea.showDependencies : true
            checkable: true
            Layout.preferredWidth: 125
            font.pixelSize: 11
            onCheckedChanged: { if(mainWindow.hasGridArea()) mainWindow.gridArea.showDependencies = checked }
        }
        Button
        {
            text: "Переносы"
            checked: mainWindow.hasGridArea() ? mainWindow.gridArea.showTaskHistory : true
            checkable: true
            Layout.preferredWidth: 95
            font.pixelSize: 11
            onCheckedChanged: { if (mainWindow.hasGridArea()) mainWindow.gridArea.showTaskHistory = checked }
        }
        Button
        {
            text: "Комментарии"
            checked: mainWindow.hasGridArea() ? mainWindow.gridArea.showComments : true
            checkable: true
            Layout.preferredWidth: 125
            font.pixelSize: 11
            onCheckedChanged: { if(mainWindow.hasGridArea()) mainWindow.gridArea.showComments = checked }
        }
        Button
        {
            text: "Праздники"
            checked: projectController ? projectController.settingsManager.showHolidays : false
            checkable: true
            Layout.preferredWidth: 105
            font.pixelSize: 11
            onCheckedChanged:
            {
                if (projectController) projectController.settingsManager.showHolidays = checked
                if (mainWindow.hasGridArea()) mainWindow.gridArea.updateData()
                if (mainWindow.calendarHeader) mainWindow.calendarHeader.refresh()
            }
        }

        // Растяжка — один раз, чтобы прижать правую группу
        Item { Layout.fillWidth: true; Layout.minimumWidth: 4 }

        Button
        {
            text: "Помощь"
            Layout.preferredWidth: 75
            font.pixelSize: 11
            onClicked: if (mainWindow.helpDialog) mainWindow.helpDialog.show()
        }
        Button
        {
            text: "О программе"
            Layout.preferredWidth: 120
            font.pixelSize: 11
            onClicked: if (mainWindow.aboutDialog) mainWindow.aboutDialog.open()
        }
    }
}
