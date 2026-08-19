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
    property var tabs: []
    property int tabCounter: 0

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
    }

    Windows.NotificationsWindow {
        id: notificationsWindow
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

        GridView {
            id: tilesGrid
            anchors.fill: parent
            anchors.margins: 20
            cellWidth: 200
            cellHeight: 150
            model: root.tiles
            visible: root.tabs.length === 0

            delegate: Rectangle {
                width: 180
                height: 120
                radius: 12
                color: Colors.surface
                border.color: Colors.border

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
                    onEntered: parent.color = Colors.buttonHover
                    onExited: parent.color = Colors.surface
                    onClicked: {
                        var tile = modelData
                        var endpoint = tile.endpoint
                        var method = tile.method || "GET"
                        var label = tile.label || "Вкладка"
                        var tileId = tile.id || "tab_" + (root.tabCounter++)

                        var component = Qt.createComponent("components/TabContent.qml")
                        if (component.status === Component.Ready) {
                            var tab = component.createObject(tabsContainer, {
                                "title": label,
                                "endpoint": endpoint,
                                "method": method,
                                "accessToken": mainWindowBridge.accessToken,
                                "tileId": tileId
                            })
                            if (tab) {
                                root.tabs.push(tab)
                                tilesGrid.visible = false
                                tabsContainer.visible = true
                                tab.loadData()
                            }
                        } else {
                            console.log("Failed to create TabContent:", component.errorString())
                        }
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

        Button {
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.margins: 10
            text: "✕ Закрыть все"
            visible: root.tabs.length > 0
            onClicked: {
                for (var i = 0; i < root.tabs.length; i++) {
                    root.tabs[i].destroy()
                }
                root.tabs = []
                tabsContainer.visible = false
                tilesGrid.visible = true
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
