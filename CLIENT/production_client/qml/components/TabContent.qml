import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0
import QtQuick.Shapes 6.0
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
    property bool canGoBack: false

    readonly property int backButtonWidth: 48
    readonly property int backButtonRadius: 8

    function cancelRequest() {
        loading = false
    }

    // ---- Кнопка «Назад» ----
    Shape {
        id: backButtonBase
        width: root.backButtonWidth
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        z: 10

        ShapePath {
            fillColor: root.canGoBack ? Colors.actionBg : Colors.actionDisabledBg
            strokeColor: "transparent"
            startX: 0
            startY: 0
            PathLine { x: backButtonBase.width - root.backButtonRadius; y: 0 }
            PathArc {
                x: backButtonBase.width; y: root.backButtonRadius
                radiusX: root.backButtonRadius; radiusY: root.backButtonRadius
            }
            PathLine { x: backButtonBase.width; y: backButtonBase.height - root.backButtonRadius }
            PathArc {
                x: backButtonBase.width - root.backButtonRadius; y: backButtonBase.height
                radiusX: root.backButtonRadius; radiusY: root.backButtonRadius
            }
            PathLine { x: 0; y: backButtonBase.height }
            PathLine { x: 0; y: 0 }
        }
    }

    Shape {
        id: backButtonHover
        width: root.backButtonWidth
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        z: 11
        opacity: (root.canGoBack && backMouse.containsMouse) ? 1.0 : 0.0

        Behavior on opacity {
            NumberAnimation { duration: 150 }
        }

        ShapePath {
            fillColor: Colors.buttonHover
            strokeColor: "transparent"
            startX: 0
            startY: 0
            PathLine { x: backButtonHover.width - root.backButtonRadius; y: 0 }
            PathArc {
                x: backButtonHover.width; y: root.backButtonRadius
                radiusX: root.backButtonRadius; radiusY: root.backButtonRadius
            }
            PathLine { x: backButtonHover.width; y: backButtonHover.height - root.backButtonRadius }
            PathArc {
                x: backButtonHover.width - root.backButtonRadius; y: backButtonHover.height
                radiusX: root.backButtonRadius; radiusY: root.backButtonRadius
            }
            PathLine { x: 0; y: backButtonHover.height }
            PathLine { x: 0; y: 0 }
        }
    }

    Canvas {
        id: backTriangle
        anchors.centerIn: backButtonBase
        width: backButtonBase.width * 0.5
        height: backButtonBase.height * 0.25
        z: 12

        property color triangleColor: root.canGoBack ? Colors.actionFg : Colors.actionDisabledFg

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            ctx.beginPath()
            ctx.moveTo(width * 0.75, 0)
            ctx.lineTo(width * 0.75, height)
            ctx.lineTo(width * 0.25, height / 2)
            ctx.closePath()
            ctx.fillStyle = triangleColor
            ctx.fill()
        }

        Connections {
            target: Colors
            function onIsDarkThemeChanged() { backTriangle.requestPaint() }
            function onActionFgChanged() { backTriangle.requestPaint() }
            function onActionDisabledFgChanged() { backTriangle.requestPaint() }
        }

        onTriangleColorChanged: requestPaint()
    }

    MouseArea {
        id: backMouse
        anchors.fill: backButtonBase
        hoverEnabled: true
        cursorShape: root.canGoBack ? Qt.PointingHandCursor : Qt.ArrowCursor
        z: 13
        onClicked: {
            if (!root.canGoBack) return
            if (!root.sessionId) return

            var payload = { "session_id": root.sessionId }
            var cbId = "back_" + Date.now()
            root.callbackId = cbId

            widgetBridge.httpRequest(
                "http://localhost:8080/workflow/back",
                "POST",
                root.accessToken,
                JSON.stringify(payload),
                cbId
            )
        }
    }

    // ---- Контент ----
    Flickable {
        id: uiFlick
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.left: backButtonBase.right
        anchors.topMargin: 10
        anchors.bottomMargin: 10
        anchors.rightMargin: 10
        anchors.leftMargin: root.backButtonRadius
        visible: !loading
        clip: true
        contentWidth: width
        contentHeight: 0
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

    Connections {
        target: renderer
        function onContentHeightChanged(height) {
            uiFlick.contentHeight = height
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
        anchors.leftMargin: root.backButtonWidth + root.backButtonRadius
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

    function renderResponse(response)
    {
        if (response.context) {
            root.sessionId = response.context.session_id || ""
            root.currentStep = response.context.current_step || ""
            root.canGoBack = response.context.can_go_back === true
        }
        if (response.title) {
            root.title = response.title
        }
        if (response.widgets && response.widgets.length > 0) {
            debugText.visible = false
            widgetBridge.renderPage(response, uiContainer)
        } else {
            debugText.text = JSON.stringify(response, null, 2)
            debugText.visible = true
        }
    }

    Connections {
        target: widgetBridge
        function onHttpResponse(id, status, data) {
            if (id === root.callbackId) {
                root.loading = false
                if (status === 200) {
                    try {
                        var response = JSON.parse(data)
                        renderResponse(response)
                    } catch (e) {
                        debugText.text = "Ошибка парсинга JSON: " + e.message
                        debugText.visible = true
                    }
                } else {
                    debugText.text = "Ошибка " + status + ": " + data
                    debugText.visible = true
                }
            }
        }
    }

    Component.onCompleted: {
        console.log("Tab created:", root.title, root.endpoint)
        loadData()
    }
}
