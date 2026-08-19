import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0
import Styles 1.0

Rectangle {
    id: root
    height: 65
    color: Colors.surface
    border.color: Colors.border
    border.width: 1
    z: 1

    property string userName: ""
    property string userEmail: ""
    property string userPosition: ""
    property int passwordDaysLeft: 0
    property bool passwordExpired: false
    property bool isDarkTheme: Colors.isDarkTheme
    property int unreadNotifications: 0

    signal logoutRequested()
    signal passwordChangeRequested()
    signal themeToggleRequested()
    signal fullScreenToggled()
    signal notificationsRequested()
    signal settingsRequested()

    RowLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 10

        ColumnLayout {
            spacing: 2
            Layout.alignment: Qt.AlignVCenter
            Layout.fillWidth: true

            Text {
                text: root.userName ? root.userName : "Пользователь"
                color: Colors.text
                font.pixelSize: 16
                font.bold: true
            }

            Text {
                text: root.userEmail
                color: Colors.textSecondary
                font.pixelSize: 12
                visible: root.userEmail !== ""
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
                    if (root.passwordDaysLeft > 30) return Colors.success
                    if (root.passwordDaysLeft > 14) return Colors.warning
                    return Colors.error
                }
                opacity: 0.9
                visible: root.passwordDaysLeft > 0

                Text {
                    id: passwordText
                    anchors.centerIn: parent
                    text: "Пароль: " + root.passwordDaysLeft + " дн."
                    color: Colors.buttonText
                    font.pixelSize: 13
                    font.bold: true
                }
            }

            Rectangle {
                anchors.centerIn: parent
                width: expiredText.implicitWidth + 40
                height: 36
                radius: 18
                color: Colors.error
                opacity: 0.9
                visible: root.passwordExpired

                Text {
                    id: expiredText
                    anchors.centerIn: parent
                    text: "Пароль просрочен!"
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
                text: root.isDarkTheme ? "☀️" : "🌙"
                font.pixelSize: 20
                flat: true
                implicitWidth: 40
                implicitHeight: 40
                onClicked: {
                    Colors.toggleTheme()
                    root.isDarkTheme = Colors.isDarkTheme
                    root.themeToggleRequested()
                }
            }

            Button {
                text: "⛶"
                font.pixelSize: 20
                flat: true
                implicitWidth: 40
                implicitHeight: 40
                onClicked: root.fullScreenToggled()
            }

            Button {
                text: "⚙️"
                font.pixelSize: 20
                flat: true
                implicitWidth: 40
                implicitHeight: 40
                onClicked: root.settingsRequested()
            }

            Button {
                text: "🔔"
                font.pixelSize: 20
                flat: true
                implicitWidth: 40
                implicitHeight: 40
                onClicked: root.notificationsRequested()

                Rectangle {
                    anchors.top: parent.top
                    anchors.right: parent.right
                    anchors.margins: -2
                    width: 18
                    height: 18
                    radius: 9
                    color: Colors.error
                    visible: root.unreadNotifications > 0

                    Text {
                        anchors.centerIn: parent
                        text: root.unreadNotifications > 99 ? "99+" : root.unreadNotifications
                        color: Colors.buttonText
                        font.pixelSize: 10
                        font.bold: true
                    }
                }
            }

            Button {
                text: "🔑"
                font.pixelSize: 18
                flat: true
                implicitWidth: 40
                implicitHeight: 40
                onClicked: root.passwordChangeRequested()
            }

            Button {
                text: "🚪"
                font.pixelSize: 18
                flat: true
                implicitWidth: 40
                implicitHeight: 40
                onClicked: root.logoutRequested()
            }
        }
    }
}
