import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0
import Styles 1.0

Window {
    id: root
    title: "Настройки"
    width: 350
    height: 380
    modality: Qt.WindowModal
    flags: Qt.Dialog | Qt.WindowCloseButtonHint

    ButtonGroup {
        id: lightGroup
    }

    ButtonGroup {
        id: darkGroup
    }

    Column {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 12

        Text {
            text: "Настройки оформления"
            font.pixelSize: 18
            font.bold: true
            color: Colors.text
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Rectangle {
            width: parent.width
            height: 1
            color: Colors.border
        }

        Text {
            text: "Выберите гамму дневной темы"
            font.pixelSize: 13
            color: Colors.text
        }

        RadioButton {
            text: "Синяя"
            checked: Colors.lightScheme === 0
            ButtonGroup.group: lightGroup
            onClicked: { Colors.setLightScheme(0) }
        }

        RadioButton {
            text: "Зеленая"
            checked: Colors.lightScheme === 1
            ButtonGroup.group: lightGroup
            onClicked: { Colors.setLightScheme(1) }
        }

        RadioButton {
            text: "Фиолетовая"
            checked: Colors.lightScheme === 2
            ButtonGroup.group: lightGroup
            onClicked: { Colors.setLightScheme(2) }
        }

        Rectangle {
            width: parent.width
            height: 1
            color: Colors.border
        }

        Text {
            text: "Выберите гамму ночной темы"
            font.pixelSize: 13
            color: Colors.text
        }

        RadioButton {
            text: "Синяя"
            checked: Colors.darkScheme === 0
            ButtonGroup.group: darkGroup
            onClicked: { Colors.setDarkScheme(0) }
        }

        RadioButton {
            text: "Зеленая"
            checked: Colors.darkScheme === 1
            ButtonGroup.group: darkGroup
            onClicked: { Colors.setDarkScheme(1) }
        }

        RadioButton {
            text: "Фиолетовая"
            checked: Colors.darkScheme === 2
            ButtonGroup.group: darkGroup
            onClicked: { Colors.setDarkScheme(2) }
        }

        Item {
            height: 10
            width: parent.width
        }

        Button {
            text: "Закрыть"
            anchors.horizontalCenter: parent.horizontalCenter
            onClicked: root.close()
            contentItem: Text {
                text: parent.text
                color: Colors.buttonText
            }
            background: Rectangle {
                color: Colors.button
                radius: 6
            }
        }
    }
}
