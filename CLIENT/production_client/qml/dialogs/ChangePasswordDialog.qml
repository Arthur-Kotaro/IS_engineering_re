import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0
import Styles 1.0

Dialog {
    id: root
    title: "Смена пароля"
    modal: true
    width: 400
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2

    signal passwordChanged()

    background: Rectangle {
        color: Colors.surface
        radius: 12
        border.color: Colors.border
    }

    header: Item {
        height: 50
        width: parent.width

        Text {
            text: "Смена пароля"
            font.pixelSize: 18
            font.bold: true
            color: Colors.text
            anchors.centerIn: parent
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        Text {
            text: "Введите текущий и новый пароль"
            color: Colors.textSecondary
            font.pixelSize: 12
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
        }

        TextField {
            id: currentPasswordField
            Layout.fillWidth: true
            placeholderText: "Текущий пароль"
            echoMode: TextField.Password
            color: Colors.text
            placeholderTextColor: Colors.textSecondary

            background: Rectangle {
                color: Colors.background
                border.color: Colors.border
                border.width: 1
                radius: 6
            }
        }

        TextField {
            id: newPasswordField
            Layout.fillWidth: true
            placeholderText: "Новый пароль"
            echoMode: TextField.Password
            color: Colors.text
            placeholderTextColor: Colors.textSecondary

            background: Rectangle {
                color: Colors.background
                border.color: Colors.border
                border.width: 1
                radius: 6
            }
        }

        TextField {
            id: confirmPasswordField
            Layout.fillWidth: true
            placeholderText: "Подтвердите пароль"
            echoMode: TextField.Password
            color: Colors.text
            placeholderTextColor: Colors.textSecondary

            background: Rectangle {
                color: Colors.background
                border.color: Colors.border
                border.width: 1
                radius: 6
            }
        }

        Text {
            id: errorText
            visible: false
            color: Colors.error
            font.pixelSize: 11
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Button {
                text: "Отмена"
                Layout.fillWidth: true
                implicitHeight: 40
                onClicked: root.close()

                background: Rectangle {
                    color: Colors.border
                    radius: 6
                }

                contentItem: Text {
                    text: parent.text
                    color: Colors.text
                    font.pixelSize: 14
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Button {
                text: "Сменить"
                Layout.fillWidth: true
                implicitHeight: 40
                enabled: newPasswordField.text.length >= 6 && newPasswordField.text === confirmPasswordField.text
                onClicked: changePassword()

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
        }
    }

    function changePassword() {
        mainWindowBridge.changePassword(currentPasswordField.text, newPasswordField.text)
    }

    Connections {
        target: mainWindowBridge
        function onPasswordChangeCompleted(success, message) {
            if (success) {
                passwordChanged()
                root.close()
            } else {
                errorText.text = message
                errorText.visible = true
            }
        }
    }
}
