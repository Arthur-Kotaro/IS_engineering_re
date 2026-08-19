import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0
import Styles 1.0

Rectangle {
    id: root
    color: Colors.background
    width: tabsContainer ? tabsContainer.width : parent.width
    height: tabsContainer ? tabsContainer.height : parent.height

    property string title: "Вкладка"
    property string endpoint: ""
    property string method: "GET"
    property string accessToken: ""
    property string tileId: ""
    property bool loading: false

    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 40
        color: Colors.surface
        border.color: Colors.border
        border.width: 1

        RowLayout {
            anchors.fill: parent
            anchors.margins: 10
            spacing: 10

            Text {
                text: root.title
                font.pixelSize: 14
                font.bold: true
                color: Colors.text
                Layout.fillWidth: true
            }

            Button {
                text: "Обновить"
                onClicked: loadData()
                flat: true
                font.pixelSize: 12
            }

            Button {
                text: "✕"
                flat: true
                font.pixelSize: 14
                onClicked: {
                    root.destroy()
                    var index = root.tabs.indexOf(root)
                    if (index !== -1) {
                        root.tabs.splice(index, 1)
                    }
                    if (root.tabs.length === 0) {
                        tabsContainer.visible = false
                        tilesGrid.visible = true
                    }
                }
            }
        }
    }

    ScrollView {
        anchors.top: parent.top
        anchors.topMargin: 40
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 10
        clip: true

        TextArea {
            id: textArea
            readOnly: true
            color: Colors.text
            font.pixelSize: 12
            font.family: "Monospace"
            background: Rectangle {
                color: Colors.surface
                border.color: Colors.border
                radius: 6
            }
            padding: 15
            wrapMode: Text.Wrap
            text: loading ? "Загрузка..." : "Готов к загрузке"
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: root.loading
        visible: root.loading
    }

    function loadData() {
        if (!root.endpoint) {
            textArea.text = "Ошибка: не указан endpoint"
            return
        }

        root.loading = true
        textArea.text = "Загрузка данных..."

        var request = new XMLHttpRequest()
        request.open(root.method, root.endpoint, true)
        request.setRequestHeader("Authorization", "Bearer " + root.accessToken)
        request.setRequestHeader("Content-Type", "application/json")

        request.onreadystatechange = function() {
            if (request.readyState === XMLHttpRequest.DONE) {
                root.loading = false
                if (request.status === 200) {
                    try {
                        var response = JSON.parse(request.responseText)
                        textArea.text = JSON.stringify(response, null, 2)
                    } catch (e) {
                        textArea.text = "Ошибка парсинга JSON: " + e.message
                    }
                } else if (request.status === 401) {
                    textArea.text = "Ошибка 401: Неавторизован. Требуется повторный вход."
                } else {
                    textArea.text = "Ошибка " + request.status + ": " + request.statusText
                }
            }
        }

        request.onerror = function() {
            root.loading = false
            textArea.text = "Ошибка сети: не удалось подключиться к серверу"
        }

        request.send()
    }

    function close() {
        root.destroy()
    }

    Component.onCompleted: {
        console.log("Tab created:", root.title, root.endpoint)
    }
}
