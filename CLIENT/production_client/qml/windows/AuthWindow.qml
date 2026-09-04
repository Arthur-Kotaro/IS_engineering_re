import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0
import Styles 1.0

Rectangle {
    id: authPage
    anchors.fill: parent
    color: Colors.background  // binding — обновляется автоматически

    property bool isLoading: false
    property string errorMessage: ""
    property bool showPassword: false

    function doLogin() {
        if (isLoading) return
        
        var email = loginField.text.trim()
        var password = passwordField.text
        
        if (email.length === 0) {
            errorMessage = "Введите email"
            return
        }
        if (password.length === 0) {
            errorMessage = "Введите пароль"
            return
        }
        
        errorMessage = ""
        isLoading = true
        
        authBridge.attemptLogin(email, password)
    }

    Component.onCompleted: {
        loginField.forceActiveFocus()
    }

    Connections {
        target: authBridge
        function onLoginSuccess() {
            isLoading = false
            errorMessage = ""
        }
        function onLoginFailed(message) {
            isLoading = false
            errorMessage = message
        }
    }

    ColumnLayout {
        anchors.centerIn: parent
        width: parent.width - 60
        spacing: 20

        Image {
            source: "qrc:/assets/images/logo.svg"
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 120
            Layout.preferredHeight: 120
            fillMode: Image.PreserveAspectFit
        }

        Text {
            text: "Engineering:re"
            color: Colors.text
            font.pixelSize: 24
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
            height: 1
            color: Colors.border
        }

        TextField {
            id: loginField
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            placeholderText: "Email"
            placeholderTextColor: Colors.textSecondary
            color: Colors.text
            selectionColor: Colors.primary
            background: Rectangle {
                color: Colors.surface
                border.color: Colors.border
                border.width: 1
                radius: 4
            }
            onAccepted: {
                if (text.length > 0 && passwordField.text.length > 0) {
                    doLogin()
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 0

            TextField {
                id: passwordField
                Layout.fillWidth: true
                Layout.preferredHeight: 40
                placeholderText: "Пароль"
                placeholderTextColor: Colors.textSecondary
                color: Colors.text
                selectionColor: Colors.primary
                echoMode: authPage.showPassword ? TextInput.Normal : TextInput.Password
                background: Rectangle {
                    color: Colors.surface
                    border.color: Colors.border
                    border.width: 1
                    radius: 4
                }
                onAccepted: {
                    if (loginField.text.length > 0 && text.length > 0) {
                        doLogin()
                    }
                }
            }

            Button {
                Layout.preferredWidth: 40
                Layout.preferredHeight: 40
                text: authPage.showPassword ? "👁️" : "👁️‍🗨️"
                flat: true
                onClicked: {
                    authPage.showPassword = !authPage.showPassword
                }
                background: Rectangle {
                    color: "transparent"
                    radius: 4
                }
                contentItem: Text {
                    text: parent.text
                    color: Colors.textSecondary
                    font.pixelSize: 18
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }

        Text {
            id: errorText
            Layout.fillWidth: true
            text: errorMessage
            color: Colors.error
            font.pixelSize: 12
            wrapMode: Text.WordWrap
            visible: errorMessage.length > 0
        }

        Button {
            id: loginButton
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            text: isLoading ? "Вход..." : "Войти"
            enabled: !isLoading
            onClicked: doLogin()
            contentItem: Text {
                text: parent.text
                color: "white"
                font.pixelSize: 14
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            background: Rectangle {
                color: "#2196F3"
                radius: 6
            }
        }

        BusyIndicator {
            Layout.alignment: Qt.AlignHCenter
            running: isLoading
            visible: isLoading
        }

        Text {
            text: "Версия 1.0.0"
            color: Colors.textSecondary
            font.pixelSize: 10
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 10
        }
    }
}
