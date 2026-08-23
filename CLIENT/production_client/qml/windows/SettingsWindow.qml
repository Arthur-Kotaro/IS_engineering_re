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
    color: Colors.background

    ButtonGroup {
        id: lightGroup
    }

    ButtonGroup {
        id: darkGroup
    }

    property bool themeChanged: false

    onThemeChangedChanged: {
        if (themeChanged) {
            updateColors()
            themeChanged = false
        }
    }

    function updateColors() {
        root.color = Colors.background
        for (var i = 0; i < radioButtons.children.length; i++) {
            var child = radioButtons.children[i]
            if (child.hasOwnProperty("contentItem")) {
                if (child.contentItem) {
                    child.contentItem.color = Colors.text
                }
            }
        }
    }

    Column {
        id: radioButtons
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
            id: light0
            text: "Синяя"
            checked: Colors.lightScheme === 0
            ButtonGroup.group: lightGroup
            onClicked: { Colors.setLightScheme(0) }
            contentItem: Text {
                text: light0.text
                color: Colors.text
                font: light0.font
                verticalAlignment: Text.AlignVCenter
                leftPadding: 40
            }
        }

        RadioButton {
            id: light1
            text: "Зеленая"
            checked: Colors.lightScheme === 1
            ButtonGroup.group: lightGroup
            onClicked: { Colors.setLightScheme(1) }
            contentItem: Text {
                text: light1.text
                color: Colors.text
                font: light1.font
                verticalAlignment: Text.AlignVCenter
                leftPadding: 40
            }
        }

        RadioButton {
            id: light2
            text: "Фиолетовая"
            checked: Colors.lightScheme === 2
            ButtonGroup.group: lightGroup
            onClicked: { Colors.setLightScheme(2) }
            contentItem: Text {
                text: light2.text
                color: Colors.text
                font: light2.font
                verticalAlignment: Text.AlignVCenter
                leftPadding: 40
            }
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
            id: dark0
            text: "Синяя"
            checked: Colors.darkScheme === 0
            ButtonGroup.group: darkGroup
            onClicked: { Colors.setDarkScheme(0) }
            contentItem: Text {
                text: dark0.text
                color: Colors.text
                font: dark0.font
                verticalAlignment: Text.AlignVCenter
                leftPadding: 40
            }
        }

        RadioButton {
            id: dark1
            text: "Зеленая"
            checked: Colors.darkScheme === 1
            ButtonGroup.group: darkGroup
            onClicked: { Colors.setDarkScheme(1) }
            contentItem: Text {
                text: dark1.text
                color: Colors.text
                font: dark1.font
                verticalAlignment: Text.AlignVCenter
                leftPadding: 40
            }
        }

        RadioButton {
            id: dark2
            text: "Фиолетовая"
            checked: Colors.darkScheme === 2
            ButtonGroup.group: darkGroup
            onClicked: { Colors.setDarkScheme(2) }
            contentItem: Text {
                text: dark2.text
                color: Colors.text
                font: dark2.font
                verticalAlignment: Text.AlignVCenter
                leftPadding: 40
            }
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
