import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0
import Styles 1.0
import "../dialogs" as Dialogs

Rectangle {
    id: root
    anchors.fill: parent
    color: Colors.background

    signal logoutRequested()
    signal themeToggleRequested()

    property var tiles: []
    property var tabs: []

    // Верхняя панель
    Rectangle {
        id: topBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 65
        color: Colors.surface
        border.color: Colors.border
        border.width: 1
        z: 1

        RowLayout {
            anchors.fill: parent
            anchors.margins: 15
            spacing: 10

            ColumnLayout {
                spacing: 2
                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: true

                Text {
                    text: mainWindowBridge ? mainWindowBridge.userName : "Пользователь"
                    color: Colors.text
                    font.pixelSize: 16
                    font.bold: true
                }

                Text {
                    text: mainWindowBridge ? mainWindowBridge.userEmail : ""
                    color: Colors.textSecondary
                    font.pixelSize: 12
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.minimumWidth: 100
                height: parent.height

                Rectangle {
                    anchors.centerIn: parent
                    width: passwordText.implicitWidth + 40
                    height: 36
                    radius: 18
                    color: {
                        if (mainWindowBridge) {
                            var days = mainWindowBridge.passwordDaysLeft
                            if (days > 30) return Colors.success
                            if (days > 14) return Colors.warning
                            return Colors.error
                        }
                        return Colors.border
                    }
                    opacity: 0.9

                    Text {
                        id: passwordText
                        anchors.centerIn: parent
                        text: mainWindowBridge ? "Пароль: " + mainWindowBridge.passwordDaysLeft + " дн." : "Пароль: --"
                        color: Colors.buttonText
                        font.pixelSize: 13
                        font.bold: true
                    }
                }
            }

            RowLayout {
                spacing: 8
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

                Button {
                    text: "🌓"
                    font.pixelSize: 20
                    flat: true
                    implicitWidth: 40
                    implicitHeight: 40
                    onClicked: root.themeToggleRequested()
                }

                Button {
                    text: "🔑"
                    font.pixelSize: 18
                    flat: true
                    implicitWidth: 40
                    implicitHeight: 40
                    onClicked: changePasswordDialog.open()
                }

                Button {
                    text: "🚪"
                    font.pixelSize: 18
                    flat: true
                    implicitWidth: 40
                    implicitHeight: 40
                    onClicked: {
                        mainWindowBridge.logout()
                        root.logoutRequested()
                    }
                }
            }
        }
    }

    Dialogs.ChangePasswordDialog {
        id: changePasswordDialog
        onPasswordChanged: mainWindowBridge.checkPasswordExpiry()
    }

    GridView {
        anchors.top: topBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 20
        cellWidth: 200
        cellHeight: 150
        model: root.tiles

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
                    console.log("Clicked:", modelData.label)
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
