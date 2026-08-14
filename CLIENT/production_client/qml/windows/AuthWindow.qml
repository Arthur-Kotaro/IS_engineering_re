import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0
import Styles 1.0

Rectangle {
    id: root
    anchors.fill: parent
    color: Colors.background

    signal loginSuccess()

    ColumnLayout {
        anchors.centerIn: parent
        width: 350
        spacing: 15

        Text {
            text: "Engineering :re"
            color: Colors.text
            font.pixelSize: 28
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }

        Text {
            text: "Вход в систему"
            color: Colors.textSecondary
            font.pixelSize: 14
            Layout.alignment: Qt.AlignHCenter
        }

        Rectangle {
            Layout.fillWidth: true
            height: 300
            radius: 12
            color: Colors.surface
            border.color: Colors.border

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 15

                TextField {
                    id: emailField
                    Layout.fillWidth: true
                    placeholderText: "Email"
                    color: Colors.text
                    background: Rectangle {
                        color: Colors.background
                        border.color: Colors.border
                        radius: 6
                    }
                }

                TextField {
                    id: passwordField
                    Layout.fillWidth: true
                    placeholderText: "Пароль"
                    echoMode: TextField.Password
                    color: Colors.text
                    background: Rectangle {
                        color: Colors.background
                        border.color: Colors.border
                        radius: 6
                    }
                    onAccepted: performLogin()
                }

                Button {
                    text: "Войти"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 40
                    onClicked: performLogin()
                    background: Rectangle {
                        color: Colors.button
                        radius: 6
                    }
                    contentItem: Text {
                        text: parent.text
                        color: Colors.buttonText
                        font.pixelSize: 14
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }

                Text {
                    id: errorText
                    visible: false
                    color: Colors.error
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                }

                BusyIndicator {
                    id: loginBusy
                    running: false
                    Layout.alignment: Qt.AlignHCenter
                    visible: running
                }
            }
        }
    }

    function performLogin() {
        loginBusy.running = true
        errorText.visible = false
        authBridge.attemptLogin(emailField.text, passwordField.text)
    }

    Connections {
        target: authBridge
        function onLoginSuccess() {
            loginBusy.running = false
            root.loginSuccess()
        }
        function onLoginFailed(message) {
            loginBusy.running = false
            errorText.text = message
            errorText.visible = true
        }
    }
}
