import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0
import QtQuick.Shapes 6.0
import Styles 1.0
import "../dialogs"

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
    readonly property int titleBarHeight: 48

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

    // ---- Заголовок страницы ----
    Rectangle {
        id: titleBar
        anchors.top: parent.top
        anchors.left: backButtonBase.right
        anchors.right: parent.right
        anchors.leftMargin: root.backButtonRadius
        anchors.rightMargin: 10
        anchors.topMargin: 10
        height: root.titleBarHeight
        color: "transparent"
        visible: root.title !== "" && !root.loading
        z: 5

        Text {
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            text: root.title
            color: Colors.text
            font.pixelSize: Typography.h4
            font.bold: true
            elide: Text.ElideRight
        }
    }

    // ---- Контент ----
    Flickable {
        id: uiFlick
        anchors.top: titleBar.visible ? titleBar.bottom : parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.left: backButtonBase.right
        anchors.topMargin: titleBar.visible ? 4 : 10
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

    // ---- Диалог ошибок валидации ----
    ValidationErrorDialog {
        id: validationErrorDialog
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
            widgetBridge.setCurrentSessionId(root.sessionId)
        }

        if (response.context && response.context.reset_form === true) {
            console.log("TabContent: reset_form=true, clearing form parameters")
            widgetBridge.clearParameters()
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

    // Парсит строку вида "Service error: {...}" -> массив ошибок [{message}, ...]
    function parseServiceError(messageText) {
        var result = []

        // Ищем первый символ '{' и последний '}'
        var startIdx = messageText.indexOf("{")
        var endIdx = messageText.lastIndexOf("}")

        if (startIdx >= 0 && endIdx > startIdx) {
            var jsonPart = messageText.substring(startIdx, endIdx + 1)
            try {
                var parsed = JSON.parse(jsonPart)
                if (parsed.detail) {
                    if (typeof parsed.detail === "string") {
                        result.push({ message: parsed.detail })
                    } else if (Array.isArray(parsed.detail)) {
                        for (var i = 0; i < parsed.detail.length; i++) {
                            var d = parsed.detail[i]
                            var loc = d.loc ? d.loc.join(".") : ""
                            var msg = d.msg || JSON.stringify(d)
                            result.push({ message: (loc ? loc + ": " : "") + msg })
                        }
                    }
                } else if (parsed.errors && parsed.errors.length > 0) {
                    for (var j = 0; j < parsed.errors.length; j++) {
                        result.push({ message: parsed.errors[j].message || JSON.stringify(parsed.errors[j]) })
                    }
                } else {
                    result.push({ message: JSON.stringify(parsed) })
                }
                return result
            } catch (e) {
                // не удалось распарсить — вернуть как есть
            }
        }

        result.push({ message: messageText })
        return result
    }

    Connections {
        target: widgetBridge

        function onHttpResponse(id, status, data) {
            // --- Ответ на загрузку страницы / workflow/back ---
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
                return
            }

            // --- Ошибка workflow-события (sendWorkflowEvent) ---
            if (id.indexOf("wf_") === 0) {
                console.log("TabContent: workflow event error, status=" + status)

                var wfTitle = "Не удалось выполнить действие"
                var wfSubtitle = ""
                var wfErrors = []

                try {
                    var wfResp = JSON.parse(data)
                    if (wfResp.errors && wfResp.errors.length > 0) {
                        wfErrors = wfResp.errors
                        wfSubtitle = wfResp.detail || ""
                    } else if (wfResp.detail) {
                        if (typeof wfResp.detail === "string") {
                            // FastAPI-обёртка: detail может содержать "Service error: {...}"
                            wfErrors = parseServiceError(wfResp.detail)
                        } else {
                            wfErrors = [{ message: JSON.stringify(wfResp.detail) }]
                        }
                    } else {
                        wfErrors = [{ message: "Ошибка " + status + ": " + data }]
                    }
                } catch (e) {
                    wfErrors = [{ message: "Ошибка " + status + ": " + data }]
                }

                validationErrorDialog.showErrors(wfTitle, wfSubtitle, wfErrors)
                return
            }

            // --- Ответ на submit-форму (старый путь, оставлен для совместимости) ---
            if (id.indexOf("submit_") === 0) {
                console.log("TabContent: submit response status=" + status)
                if (status === 200) {
                    try {
                        var successResp = JSON.parse(data)
                        console.log("TabContent: created user_id="
                                    + (successResp.user ? successResp.user.user_id : "?"))
                    } catch (e) {
                        console.log("TabContent: success parse error: " + e)
                    }
                    return
                }

                var title = "Не удалось создать пользователя"
                var subtitle = ""
                var errors = []

                try {
                    var errResp = JSON.parse(data)

                    if (errResp.errors && errResp.errors.length > 0) {
                        errors = errResp.errors
                        subtitle = errResp.detail || ""
                    } else if (errResp.detail) {
                        if (typeof errResp.detail === "string") {
                            errors = parseServiceError(errResp.detail)
                        } else if (Array.isArray(errResp.detail)) {
                            for (var i = 0; i < errResp.detail.length; i++) {
                                var d = errResp.detail[i]
                                var loc = d.loc ? d.loc.join(".") : ""
                                var msg = d.msg || ""
                                errors.push({ message: (loc ? loc + ": " : "") + msg })
                            }
                        }
                    } else {
                        errors = [{ message: "Неизвестная ошибка. Код " + status }]
                    }
                } catch (e) {
                    errors = [{ message: "Ошибка " + status + ": " + data }]
                }

                validationErrorDialog.showErrors(title, subtitle, errors)
            }
        }

        function onWorkflowEventCompleted(response) {
            console.log("TabContent: workflow event completed")
            renderResponse(response)
        }
    }

    Component.onCompleted: {
        console.log("Tab created:", root.title, root.endpoint)
        loadData()
    }
}
