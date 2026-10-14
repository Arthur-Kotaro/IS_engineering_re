import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0
import Styles 1.0

Dialog {
    id: root

    property string dialogTitle: "Ошибка"
    property string dialogSubtitle: ""
    property var errorItems: []   // [{field, code, message}]

    title: dialogTitle
    modal: true
    standardButtons: Dialog.NoButton
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    width: 520
    height: Math.min(600, contentColumn.implicitHeight + 120)

    // Центрирование относительно parent-окна
    x: parent ? Math.round((parent.width - width) / 2) : 0
    y: parent ? Math.round((parent.height - height) / 2) : 0

    background: Rectangle {
        color: Colors.surface
        border.color: Colors.border
        border.width: 1
        radius: 8
    }

    header: Rectangle {
        color: "transparent"
        height: headerColumn.implicitHeight + 32

        ColumnLayout {
            id: headerColumn
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.margins: 16
            spacing: 6

            Text {
                Layout.fillWidth: true
                text: root.dialogTitle
                color: Colors.error
                font.pixelSize: Typography.h4
                font.bold: true
                wrapMode: Text.WordWrap
            }

            Text {
                Layout.fillWidth: true
                visible: root.dialogSubtitle !== ""
                text: root.dialogSubtitle
                color: Colors.textSecondary
                font.pixelSize: Typography.body1
                wrapMode: Text.WordWrap
            }
        }
    }

    contentItem: ColumnLayout {
        id: contentColumn
        spacing: 8
        anchors.margins: 16

        ScrollView {
            Layout.fillWidth: true
            Layout.preferredHeight: Math.min(errorList.implicitHeight, 400)
            clip: true
            ScrollBar.vertical.policy: ScrollBar.AsNeeded

            ColumnLayout {
                id: errorList
                width: parent.width
                spacing: 10

                Repeater {
                    model: root.errorItems

                    delegate: RowLayout {
                        Layout.fillWidth: true
                        spacing: 8

                        Rectangle {
                            width: 6
                            height: 6
                            radius: 3
                            color: Colors.error
                            Layout.alignment: Qt.AlignTop
                            Layout.topMargin: 8
                        }

                        Text {
                            Layout.fillWidth: true
                            text: modelData.message || modelData
                            color: Colors.text
                            font.pixelSize: Typography.body1
                            wrapMode: Text.WordWrap
                        }
                    }
                }
            }
        }

        Item { Layout.fillHeight: true }

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: 8

            Item { Layout.fillWidth: true }

            Button {
                text: "Понятно"
                implicitWidth: 120
                implicitHeight: 36
                onClicked: root.close()

                background: Rectangle {
                    color: parent.hovered ? Colors.buttonHover : Colors.button
                    radius: 6
                    Behavior on color { ColorAnimation { duration: 150 } }
                }

                contentItem: Text {
                    text: parent.text
                    color: Colors.buttonText
                    font.pixelSize: Typography.button
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }
        }
    }

    // Удобный метод: открыть с массивом errors
    function showErrors(title, subtitle, errors) {
        root.dialogTitle = title
        root.dialogSubtitle = subtitle
        root.errorItems = errors || []
        root.open()
    }

    // Удобный метод: открыть с одной ошибкой
    function showMessage(title, message) {
        root.dialogTitle = title
        root.dialogSubtitle = ""
        root.errorItems = [{ message: message }]
        root.open()
    }
}
