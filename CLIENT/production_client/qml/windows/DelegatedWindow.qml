import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0
import Styles 1.0
import "../components" as Components

Window {
    id: root
    width: 1024
    height: 768
    visible: true
    title: "Engineering :re — режим делегирования"

    property string donorName: ""
    property string accessToken: ""
    property string impersonationId: ""

    property var tiles: []
    property var tabObjects: []
    property int tabCounter: 0
    property int activeTabIndex: 0

    color: Colors.background

    ListModel { id: tabsModel }

    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 40
        color: Colors.warning
        z: 100

        RowLayout {
            anchors.fill: parent
            anchors.margins: 10
            Text {
                text: "🎭 РЕЖИМ ДЕЛЕГИРОВАНИЯ: " + root.donorName
                color: "white"
                font.pixelSize: 14
                font.bold: true
                Layout.fillWidth: true
            }
            Button {
                text: "Выйти из режима"
                onClicked: {
                    widgetBridge.httpRequest(
                        "http://localhost:8080/internal/auth/impersonation/close",
                        "POST",
                        root.accessToken,
                        JSON.stringify({"impersonation_id": root.impersonationId}),
                        "close_impersonation"
                    )
                    root.close()
                }
            }
        }
    }

    Components.TopBar {
        id: topBar
        anchors.top: parent.top
        anchors.topMargin: 40
        anchors.left: parent.left
        anchors.right: parent.right
        userName: root.donorName
        userEmail: ""
        userPosition: "Действие от имени"
        passwordDaysLeft: 0
        passwordExpired: false
        unreadNotifications: 0
    }

    GridView {
        id: tilesGrid
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
            border.color: Colors.button
            border.width: 1

            Text {
                anchors.centerIn: parent
                text: modelData.label || "Плитка"
                color: Colors.text
                font.pixelSize: 14
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    console.log("Delegated tile clicked:", modelData.label)
                }
            }
        }
    }

    Component.onCompleted: {
        var request = new XMLHttpRequest()
        request.open("GET", "http://localhost:8080/api/v1/navigation/dashboard", true)
        request.setRequestHeader("Authorization", "Bearer " + root.accessToken)
        request.onreadystatechange = function() {
            if (request.readyState === XMLHttpRequest.DONE && request.status === 200) {
                try {
                    var data = JSON.parse(request.responseText)
                    root.tiles = data.tiles || []
                } catch (e) { console.log("parse error:", e) }
            }
        }
        request.send()
    }
}
