import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0
import Styles 1.0

Window {
    id: root
    title: "Настройки"
    width: 420
    height: 580
    modality: Qt.WindowModal
    flags: Qt.Dialog | Qt.WindowCloseButtonHint
    color: Colors.background

    property int tempFontSize: configManager.fontSize
    property string tempLightScheme: configManager.lightScheme
    property string tempDarkScheme: configManager.darkScheme

    ButtonGroup {
        id: lightGroup
    }

    ButtonGroup {
        id: darkGroup
    }

    function applySettings() {
        configManager.setFontSize(tempFontSize)
        configManager.setLightScheme(tempLightScheme)
        configManager.setDarkScheme(tempDarkScheme)
        
        if (Colors.isDarkTheme) {
            Colors.darkScheme = tempDarkScheme === "blue" ? 0 : tempDarkScheme === "green" ? 1 : 2
        } else {
            Colors.lightScheme = tempLightScheme === "blue" ? 0 : tempLightScheme === "green" ? 1 : 2
        }
        Colors.updateColors()
        appCore.applyFontSize(tempFontSize)
        root.close()
    }

    function cancelSettings() {
        root.close()
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
            text: "Гамма для светлой темы"
            font.pixelSize: 13
            color: Colors.text
        }

        RadioButton {
            id: light0
            text: "Синяя"
            ButtonGroup.group: lightGroup
            checked: tempLightScheme === "blue"
            onClicked: { tempLightScheme = "blue" }
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
            ButtonGroup.group: lightGroup
            checked: tempLightScheme === "green"
            onClicked: { tempLightScheme = "green" }
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
            ButtonGroup.group: lightGroup
            checked: tempLightScheme === "purple"
            onClicked: { tempLightScheme = "purple" }
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
            text: "Гамма для тёмной темы"
            font.pixelSize: 13
            color: Colors.text
        }

        RadioButton {
            id: dark0
            text: "Синяя"
            ButtonGroup.group: darkGroup
            checked: tempDarkScheme === "blue"
            onClicked: { tempDarkScheme = "blue" }
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
            ButtonGroup.group: darkGroup
            checked: tempDarkScheme === "green"
            onClicked: { tempDarkScheme = "green" }
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
            ButtonGroup.group: darkGroup
            checked: tempDarkScheme === "purple"
            onClicked: { tempDarkScheme = "purple" }
            contentItem: Text {
                text: dark2.text
                color: Colors.text
                font: dark2.font
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
            text: "Размер шрифта"
            font.pixelSize: 13
            color: Colors.text
        }

        RowLayout {
            width: parent.width
            spacing: 10

            Slider {
                id: fontSizeSlider
                from: 10
                to: 20
                value: tempFontSize
                stepSize: 1
                Layout.fillWidth: true
                onValueChanged: {
                    tempFontSize = value
                    fontSizeLabel.text = value.toFixed(0)
                    appCore.applyFontSize(value)
                }
            }

            Text {
                id: fontSizeLabel
                text: tempFontSize.toFixed(0)
                font.pixelSize: 14
                color: Colors.text
                Layout.preferredWidth: 30
                horizontalAlignment: Text.AlignHCenter
            }
        }

        Item {
            height: 10
            width: parent.width
        }

        RowLayout {
            width: parent.width
            spacing: 10
            anchors.horizontalCenter: parent.horizontalCenter

            Button {
                text: "Отмена"
                Layout.preferredWidth: 120
                Layout.preferredHeight: 40
                onClicked: cancelSettings()
                contentItem: Text {
                    text: parent.text
                    color: Colors.buttonText
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                background: Rectangle {
                    color: Colors.button
                    radius: 6
                }
            }

            Button {
                text: "Применить"
                Layout.preferredWidth: 120
                Layout.preferredHeight: 40
                onClicked: applySettings()
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
        }
    }

    Component.onCompleted: {
        tempFontSize = configManager.fontSize
        tempLightScheme = configManager.lightScheme
        tempDarkScheme = configManager.darkScheme
        fontSizeLabel.text = tempFontSize.toFixed(0)
    }
}
