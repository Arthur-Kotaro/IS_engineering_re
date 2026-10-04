import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0
import Styles 1.0

Window {
    id: root
    title: "Войти как ..."
    width: 700
    height: 600
    modality: Qt.WindowModal
    flags: Qt.Dialog | Qt.WindowCloseButtonHint
    color: Colors.background

    signal donorSelected(int donorId, string donorName)

    property var delegationDonors: []
    property var subordinateDonors: []
    property bool loading: false
    property string errorMessage: ""

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15

        Text {
            text: "Выберите пользователя, от имени которого хотите работать"
            color: Colors.text
            font.pixelSize: 16
            font.bold: true
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
        }

        Text {
            visible: root.errorMessage !== ""
            text: root.errorMessage
            color: Colors.error
            font.pixelSize: 12
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Colors.surface
            border.color: Colors.border
            radius: 6

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 10

                Text {
                    text: "Временные делегирования"
                    color: Colors.text
                    font.pixelSize: 14
                    font.bold: true
                }

                ListView {
                    id: delegationList
                    Layout.fillWidth: true
                    Layout.preferredHeight: 150
                    clip: true
                    model: root.delegationDonors
                    delegate: Rectangle {
                        width: delegationList.width
                        height: 36
                        color: delegateMouse.containsMouse ? Colors.buttonHover : "transparent"
                        radius: 4
                        MouseArea {
                            id: delegateMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.donorSelected(modelData.user_id, modelData.full_name || modelData.user_name)
                        }
                        Text {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            verticalAlignment: Text.AlignVCenter
                            text: (modelData.full_name || modelData.user_name) + "  (делегирование)"
                            color: Colors.text
                            font.pixelSize: 13
                        }
                    }
                }

                Text {
                    visible: root.delegationDonors.length === 0
                    text: "Временных делегирований нет"
                    color: Colors.textSecondary
                    font.pixelSize: 12
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                }

                Rectangle { Layout.fillWidth: true; height: 1; color: Colors.border }

                Text {
                    text: "Мои подчинённые"
                    color: Colors.text
                    font.pixelSize: 14
                    font.bold: true
                }

                ListView {
                    id: subList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: root.subordinateDonors
                    delegate: Rectangle {
                        width: subList.width
                        height: 36
                        color: subMouse.containsMouse ? Colors.buttonHover : "transparent"
                        radius: 4
                        MouseArea {
                            id: subMouse
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.donorSelected(modelData.user_id, modelData.full_name || modelData.user_name)
                        }
                        Text {
                            anchors.fill: parent
                            anchors.leftMargin: 10
                            verticalAlignment: Text.AlignVCenter
                            text: (modelData.full_name || modelData.user_name) + "  (подчинённый)"
                            color: Colors.text
                            font.pixelSize: 13
                        }
                    }
                }

                Text {
                    visible: root.subordinateDonors.length === 0
                    text: "Подчинённых нет"
                    color: Colors.textSecondary
                    font.pixelSize: 12
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }

        BusyIndicator {
            running: root.loading
            visible: root.loading
            Layout.alignment: Qt.AlignHCenter
        }

        RowLayout {
            Layout.fillWidth: true
            Item { Layout.fillWidth: true }
            Button {
                text: "Отмена"
                onClicked: root.close()
            }
        }
    }
}
