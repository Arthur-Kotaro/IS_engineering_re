import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0
import Styles 1.0

Rectangle {
    id: root
    color: Colors.background
    anchors.fill: parent
    visible: true

    property string title: "Вкладка"
    property string endpoint: ""
    property string method: "GET"
    property string accessToken: ""
    property string tileId: ""
    property bool loading: false
    property string callbackId: ""

    function cancelRequest() {
        loading = false
        textArea.text = "Запрос отменен"
    }

    ScrollView {
        anchors.fill: parent
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
            text: "Ожидание данных..."
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

        var url = root.endpoint
        if (!url.startsWith("http://") && !url.startsWith("https://")) {
            url = "http://localhost:8080" + url
        }

        callbackId = "tab_" + tileId + "_" + Date.now()
        widgetBridge.httpRequest(url, root.method, root.accessToken, "", callbackId)
    }

    Connections {
        target: widgetBridge
        function onHttpResponse(id, status, data) {
            if (id !== root.callbackId) return
            root.loading = false
            if (status === 200) {
                try {
                    var response = JSON.parse(data)
                    textArea.text = JSON.stringify(response, null, 2)
                } catch (e) {
                    textArea.text = "Ошибка парсинга JSON: " + e.message
                }
            } else if (status === 401) {
                textArea.text = "Ошибка 401: Неавторизован"
            } else {
                textArea.text = "Ошибка " + status + ": " + data
            }
        }
    }

    Component.onCompleted: {
        console.log("Tab created:", root.title, root.endpoint)
    }
}
