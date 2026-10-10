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
    property string sessionId: ""
    property string currentStep: ""

    function cancelRequest() {
        loading = false
    }

        Flickable {
        id: uiFlick
        anchors.fill: parent
        anchors.margins: 10
        visible: !loading
        clip: true
        contentWidth: width
        contentHeight: uiContainer.childrenRect.height
        boundsBehavior: Flickable.StopAtBounds
        flickableDirection: Flickable.VerticalFlick

        ScrollBar.vertical: ScrollBar {
            policy: ScrollBar.AsNeeded
        }

        Item {
            id: uiContainer
            width: uiFlick.width
            height: childrenRect.height
        }
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

    function loadData()
    {
        if (!root.endpoint)
        {
            debugText.text = "Ошибка: не указан endpoint"
            debugText.visible = true
            return
        }

        root.loading = true
        debugText.visible = false

        if (root.accessToken !== "") {
            widgetBridge.setAccessToken(root.accessToken)
        }

        var url = root.endpoint
        if (!url.startsWith("http://") && !url.startsWith("https://")) {
            url = "http://localhost:8080" + url
        }

        console.log("TabContent: loadData", url)
        callbackId = "tab_" + tileId + "_" + Date.now()
        widgetBridge.httpRequest(url, root.method, root.accessToken, "", callbackId)
    }

    function sendWorkflowEvent(widgetId, eventType, data) {
        if (!root.sessionId) {
            console.log("No session_id — cannot send workflow event")
            return
        }
        var payload = {
            "session_id": root.sessionId,
            "widget_id": widgetId,
            "event_type": eventType,
            "data": data || {}
        }
        var cbId = "wf_event_" + Date.now()
        widgetBridge.httpRequest(
            "http://localhost:8080/workflow/event",
            "POST",
            root.accessToken,
            JSON.stringify(payload),
            cbId
        )
    }

    Connections {
        target: widgetBridge
        function onHttpResponse(id, status, data) {
            if (id === root.callbackId) {
                root.loading = false
                if (status === 200) {
                    try {
                        var response = JSON.parse(data)
                        if (response.context) {
                            root.sessionId = response.context.session_id || ""
                            root.currentStep = response.context.current_step || ""
                        }
                        if (response.widgets && response.widgets.length > 0) {
                            debugText.visible = false
                            widgetBridge.renderPage(response, uiContainer)
                        } else {
                            debugText.text = JSON.stringify(response, null, 2)
                            debugText.visible = true
                        }
                    } catch (e) {
                        debugText.text = "Ошибка парсинга JSON: " + e.message
                        debugText.visible = true
                    }
                } else {
                    debugText.text = "Ошибка " + status + ": " + data
                    debugText.visible = true
                }
            } else if (id.indexOf("wf_event_") === 0) {
                root.loading = false
                if (status === 200) {
                    try {
                        var resp = JSON.parse(data)
                        if (resp.context) {
                            root.sessionId = resp.context.session_id || root.sessionId
                            root.currentStep = resp.context.current_step || ""
                        }
                        widgetBridge.renderPage(resp, uiContainer)
                    } catch (e) {
                        console.log("workflow event parse error:", e)
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        console.log("Tab created:", root.title, root.endpoint)
        loadData()
    }
}
