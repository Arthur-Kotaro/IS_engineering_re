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
    }

    Item {
        id: uiContainer
        anchors.fill: parent
        anchors.margins: 10
        visible: !loading
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: root.loading
        visible: root.loading
    }

    TextArea {
        id: debugText
        anchors.fill: parent
        anchors.margins: 10
        readOnly: true
        color: Colors.text
        font.pixelSize: 11
        font.family: "Monospace"
        background: Rectangle {
            color: Colors.surface
            border.color: Colors.border
            radius: 6
        }
        padding: 10
        wrapMode: Text.Wrap
        visible: false
        text: "Ожидание данных..."
    }

    function loadData() {
        if (!root.endpoint) {
            debugText.text = "Ошибка: не указан endpoint"
            debugText.visible = true
            return
        }

        root.loading = true
        debugText.text = "Загрузка данных..."
        debugText.visible = false

        var url = root.endpoint
        if (!url.startsWith("http://") && !url.startsWith("https://")) {
            url = "http://localhost:8080" + url
        }

        console.log("TabContent: loadData", url)
        callbackId = "tab_" + tileId + "_" + Date.now()
        widgetBridge.httpRequest(url, root.method, root.accessToken, "", callbackId)
    }

    Connections {
        target: widgetBridge
        function onHttpResponse(id, status, data) {
            if (id !== root.callbackId) return
            root.loading = false

            console.log("TabContent: onHttpResponse", id, status)

            if (status === 200) {
                try {
                    var response = JSON.parse(data)
                    console.log("TabContent: response keys:", Object.keys(response))
                    console.log("TabContent: has widgets?", response.widgets ? response.widgets.length : 0)

                    if (response.widgets && response.widgets.length > 0) {
                        console.log("TabContent: Rendering UI...")
                        debugText.visible = false
                        widgetBridge.renderPage(response, uiContainer)
                        console.log("TabContent: UI rendered for:", root.title)
                    } else {
                        console.log("TabContent: No widgets, showing raw JSON")
                        debugText.text = JSON.stringify(response, null, 2)
                        debugText.visible = true
                    }
                } catch (e) {
                    console.log("TabContent: JSON parse error:", e.message)
                    debugText.text = "Ошибка парсинга JSON: " + e.message
                    debugText.visible = true
                }
            } else if (status === 401) {
                debugText.text = "Ошибка 401: " + data
                debugText.visible = true
            } else {
                debugText.text = "Ошибка " + status + ": " + data
                debugText.visible = true
            }
        }
    }

    Component.onCompleted: {
        console.log("Tab created:", root.title, root.endpoint)
        loadData()
    }
}
