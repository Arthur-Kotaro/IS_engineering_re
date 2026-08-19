import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0
import Styles 1.0
import "../components" as Components
import "../dialogs" as Dialogs
import "../windows" as Windows

Rectangle {
    id: root
    anchors.fill: parent
    color: Colors.background

    signal logoutRequested()
    signal themeToggleRequested()
    signal fullScreenToggled()

    property var tiles: []
    property var tabObjects: []
    property int tabCounter: 0
    property int activeTabIndex: 0

    ListModel {
        id: tabsModel
    }

    onThemeToggleRequested: {
        Colors.toggleTheme()
        topBar.isDarkTheme = Colors.isDarkTheme
        root.color = Colors.background
    }

    function addTab(title, endpoint, method, accessToken, tileId) {
        var component = Qt.createComponent("../components/TabContent.qml")
        if (component.status === Component.Ready) {
            var tab = component.createObject(tabsContainer, {
                "title": title,
                "endpoint": endpoint,
                "method": method,
                "accessToken": accessToken,
                "tileId": tileId,
                "visible": true
            })
            if (tab) {
                tabsModel.append({"title": title})
                tabObjects.push(tab)
                activeTabIndex = tabsModel.count
                tilesGrid.visible = false
                tabsContainer.visible = true
                tab.loadData()
            }
        } else {
            console.log("Failed to create TabContent:", component.errorString())
        }
    }

    function closeTab(index) {
        if (index < 0 || index >= tabObjects.length) return
        if (tabObjects[index].loading) {
            tabObjects[index].cancelRequest()
        }
        tabObjects[index].destroy()
        tabObjects.splice(index, 1)
        tabsModel.remove(index)
        if (activeTabIndex === index + 1) {
            activeTabIndex = 0
            tilesGrid.visible = true
            tabsContainer.visible = false
        } else if (activeTabIndex > index + 1) {
            activeTabIndex = activeTabIndex - 1
        }
    }

    function switchTab(index) {
        activeTabIndex = index + 1
        tilesGrid.visible = false
        tabsContainer.visible = true
        for (var i = 0; i < tabsContainer.children.length; i++) {
            if (tabsContainer.children[i] && tabsContainer.children[i].hasOwnProperty("visible")) {
                tabsContainer.children[i].visible = (i === index)
            }
        }
    }

    Components.TopBar {
        id: topBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        userName: mainWindowBridge ? mainWindowBridge.userName : ""
        userEmail: mainWindowBridge ? mainWindowBridge.userEmail : ""
        userPosition: mainWindowBridge ? mainWindowBridge.userPosition : ""
        passwordDaysLeft: mainWindowBridge ? mainWindowBridge.passwordDaysLeft : 0
        passwordExpired: mainWindowBridge ? mainWindowBridge.passwordExpired : false
        unreadNotifications: 0

        onLogoutRequested: {
            mainWindowBridge.logout()
            root.logoutRequested()
        }

        onPasswordChangeRequested: {
            changePasswordDialog.open()
        }

        onThemeToggleRequested: {
            root.themeToggleRequested()
        }

        onFullScreenToggled: {
            root.fullScreenToggled()
        }

        onNotificationsRequested: {
            notificationsWindow.show()
            notificationsWindow.raise()
            notificationsWindow.requestActivate()
        }

        onSettingsRequested: {
            settingsWindow.show()
            settingsWindow.raise()
            settingsWindow.requestActivate()
        }
    }

    Windows.NotificationsWindow {
        id: notificationsWindow
    }

    Windows.SettingsWindow {
        id: settingsWindow
    }

    Dialogs.ChangePasswordDialog {
        id: changePasswordDialog
        onPasswordChanged: mainWindowBridge.checkPasswordExpiry()
    }

    Rectangle {
        id: contentArea
        anchors.top: topBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        color: Colors.background

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            Rectangle {
                id: tabBar
                Layout.fillWidth: true
                height: 36
                color: Colors.surface
                border.color: Colors.border
                border.width: 1

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 5
                    spacing: 2

                    Rectangle {
                        id: tilesTab
                        height: 28
                        width: 100
                        radius: 4
                        color: activeTabIndex === 0 ? Colors.primary : Colors.surface
                        border.color: activeTabIndex === 0 ? Colors.button : Colors.border
                        border.width: 1

                        Text {
                            anchors.centerIn: parent
                            text: "🏠 Главная"
                            color: activeTabIndex === 0 ? Colors.buttonText : Colors.text
                            font.pixelSize: 12
                            font.bold: activeTabIndex === 0
                        }

                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                activeTabIndex = 0
                                tilesGrid.visible = true
                                tabsContainer.visible = false
                            }
                        }
                    }

                    Repeater {
                        id: tabsRepeater
                        model: tabsModel

                        delegate: Rectangle {
                            id: tabDelegate
                            height: 28
                            width: Math.max(80, tabTitle.implicitWidth + 50)
                            radius: 4
                            color: activeTabIndex === index + 1 ? Colors.primary : Colors.surface
                            border.color: activeTabIndex === index + 1 ? Colors.button : Colors.border
                            border.width: 1

                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 6
                                spacing: 4

                                Text {
                                    id: tabTitle
                                    text: model.title || "Вкладка"
                                    color: activeTabIndex === index + 1 ? Colors.buttonText : Colors.text
                                    font.pixelSize: 11
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }

                                Rectangle {
                                    width: 16
                                    height: 16
                                    radius: 3
                                    color: "transparent"

                                    Text {
                                        anchors.centerIn: parent
                                        text: "✕"
                                        font.pixelSize: 10
                                        color: activeTabIndex === index + 1 ? Colors.buttonText : Colors.textSecondary
                                    }

                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            root.closeTab(index)
                                        }
                                    }
                                }
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    root.switchTab(index)
                                }
                            }
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: Colors.background

                GridView {
                    id: tilesGrid
                    anchors.fill: parent
                    anchors.margins: 20
                    cellWidth: 200
                    cellHeight: 150
                    model: root.tiles
                    visible: true

                    delegate: Rectangle {
                        id: delegateRoot
                        width: 180
                        height: 120
                        radius: 12
                        color: Colors.surface
                        border.color: Colors.border
                        border.width: 1

                        Behavior on color {
                            ColorAnimation { duration: 150 }
                        }

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 5

                            Text {
                                text: modelData.label || "Плитка"
                                color: Colors.text
                                font.pixelSize: 14
                                font.bold: true
                                horizontalAlignment: Text.AlignHCenter
                                Layout.fillWidth: true
                                wrapMode: Text.WordWrap
                            }

                            Rectangle {
                                visible: modelData.badge_count > 0
                                anchors.top: parent.top
                                anchors.right: parent.right
                                anchors.margins: -5
                                width: 24
                                height: 24
                                radius: 12
                                color: Colors.error

                                Text {
                                    anchors.centerIn: parent
                                    text: modelData.badge_count
                                    color: Colors.buttonText
                                    font.pixelSize: 11
                                    font.bold: true
                                }
                            }
                        }

                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onEntered: {
                                delegateRoot.color = Colors.buttonHover
                            }
                            onExited: {
                                delegateRoot.color = Colors.surface
                            }
                            onClicked: {
                                var tile = modelData
                                var endpoint = tile.endpoint
                                var method = tile.method || "GET"
                                var label = tile.label || "Вкладка"
                                var tileId = tile.id || "tab_" + (root.tabCounter++)

                                root.addTab(label, endpoint, method, mainWindowBridge.accessToken, tileId)
                            }
                        }
                    }
                }

                Item {
                    id: tabsContainer
                    anchors.fill: parent
                    visible: false
                    clip: true
                }
            }
        }
    }

    Component.onCompleted: {
        mainWindowBridge.loadTiles()
    }

    Connections {
        target: mainWindowBridge
        function onTilesLoaded(tilesList) {
            root.tiles = tilesList
            console.log("Tiles loaded:", tilesList.length)
        }
    }
}
