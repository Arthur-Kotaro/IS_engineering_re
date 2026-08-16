import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0

ApplicationWindow {
    id: root
    width: 1400
    height: 1000
    visible: true
    title: "UI Renderer Preview"

    property string currentFile: ""

    menuBar: MenuBar {
        Menu {
            title: "Файл"
            MenuItem {
                text: "Открыть..."
                onTriggered: {
                    if (previewBridge) {
                        previewBridge.openFileDialog()
                    }
                }
            }
            MenuItem {
                text: "Перезагрузить (Ctrl+R)"
                onTriggered: {
                    if (previewBridge) {
                        previewBridge.reload()
                    }
                }
            }
            MenuSeparator {}
            MenuItem {
                text: "Выход"
                onTriggered: Qt.quit()
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        RowLayout {
            spacing: 10
            Layout.fillWidth: true
            Layout.preferredHeight: 50

            Text {
                text: "Файл:"
                color: "#888"
                font.pixelSize: 12
                Layout.preferredWidth: 40
            }

            Text {
                id: fileNameText
                text: previewBridge ? previewBridge.currentFile || "не выбран" : "не выбран"
                color: "#00bcd4"
                font.pixelSize: 12
                font.bold: true
                Layout.fillWidth: true
                elide: Text.ElideMiddle
            }

            Button {
                text: "📂 Открыть"
                onClicked: {
                    if (previewBridge) {
                        previewBridge.openFileDialog()
                    }
                }
                Layout.preferredHeight: 36
                Layout.preferredWidth: 100
            }

            Button {
                text: "⟳ Обновить"
                onClicked: {
                    if (previewBridge) {
                        previewBridge.reload()
                    }
                }
                Layout.preferredHeight: 36
                Layout.preferredWidth: 100
            }

            ComboBox {
                id: sampleCombo
                model: previewBridge ? previewBridge.sampleFiles : []
                onActivated: function(index) {
                    if (previewBridge && model.length > 0) {
                        previewBridge.loadFile(model[index])
                    }
                }
                Layout.preferredWidth: 350
                Layout.preferredHeight: 36

                displayText: {
                    if (currentIndex >= 0 && model.length > 0) {
                        var path = model[currentIndex]
                        var parts = path.split("/")
                        return parts[parts.length - 1]
                    }
                    return "Выберите образец..."
                }

                contentItem: Text {
                    text: sampleCombo.displayText
                    color: "#ffffff"
                    font.pixelSize: 12
                    elide: Text.ElideRight
                    verticalAlignment: Text.AlignVCenter
                }

                delegate: ItemDelegate {
                    width: sampleCombo.width
                    height: 30
                    contentItem: Text {
                        text: modelData
                        color: "#000000"
                        font.pixelSize: 11
                        elide: Text.ElideLeft
                        verticalAlignment: Text.AlignVCenter
                    }
                    background: Rectangle {
                        color: parent.highlighted ? "#d0d0d0" : "#ffffff"
                    }
                }

                popup.width: 500
                popup.background: Rectangle {
                    color: "#ffffff"
                    border.color: "#888"
                    border.width: 1
                }

                indicator: Rectangle {
                    implicitWidth: 20
                    implicitHeight: 20
                    color: "transparent"
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    Text {
                        anchors.centerIn: parent
                        text: "▼"
                        color: "#ffffff"
                        font.pixelSize: 12
                    }
                }

                background: Rectangle {
                    color: "#2d2d2d"
                    border.color: "#555"
                    border.width: 1
                    radius: 4
                }
            }
        }

        Rectangle {
            id: statusBar
            height: 28
            Layout.fillWidth: true
            color: "#2d2d2d"
            radius: 4

            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 15

                Text {
                    id: statusText
                    text: "Готов"
                    color: "#aaa"
                    font.pixelSize: 11
                    Layout.fillWidth: true
                }

                Text {
                    id: widgetCountText
                    text: ""
                    color: "#666"
                    font.pixelSize: 10
                }
            }
        }

        // ============================================================
        // ScrollView — ФИКСИРОВАННАЯ ВЫСОТА через количество виджетов
        // ============================================================
        ScrollView {
            id: scrollView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            ScrollBar.vertical.policy: ScrollBar.AlwaysOn
            ScrollBar.horizontal.policy: ScrollBar.AsNeeded

            background: Rectangle {
                color: "#121212"
            }

            Item {
                id: renderContainer
                width: scrollView.width - 20
                // Высота = количество виджетов * 100 + запас
                height: Math.max(containerHeight, scrollView.height - 10)
                objectName: "renderContainer"

                property int containerHeight: dynamicContent.children.length * 120 + 100

                ColumnLayout {
                    id: dynamicContent
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 12
                    height: childrenRect.height
                }

                // Обновляем высоту при изменении количества детей
                onContainerHeightChanged: {
                    var newH = Math.max(containerHeight, scrollView.height - 10)
                    renderContainer.height = newH
                    console.log("=== HEIGHT UPDATED ===")
                    console.log("children count:", dynamicContent.children.length)
                    console.log("containerHeight:", containerHeight)
                    console.log("newHeight:", newH)
                    console.log("======================")
                }
            }
        }

        Rectangle {
            id: loadingOverlay
            anchors.fill: parent
            color: "#121212"
            opacity: 0.7
            visible: false
            z: 100

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 20

                BusyIndicator {
                    running: parent.parent.visible
                    Layout.alignment: Qt.AlignHCenter
                }

                Text {
                    text: "Загрузка..."
                    color: "#aaa"
                    font.pixelSize: 14
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }
    }

    Connections {
        target: previewBridge
        enabled: previewBridge !== null

        function onFileChanged(path) {
            fileNameText.text = path
            root.currentFile = path
            statusText.text = "Загружен: " + path

            var samples = previewBridge.sampleFiles
            for (var i = 0; i < samples.length; i++) {
                if (samples[i] === path) {
                    sampleCombo.currentIndex = i
                    break
                }
            }
        }

        function onRenderComplete() {
            statusText.text = "Рендеринг завершён успешно"
            loadingOverlay.visible = false

            // Принудительно обновляем высоту
            var newH = Math.max(renderContainer.containerHeight, scrollView.height - 10)
            renderContainer.height = newH
        }

        function onErrorOccurred(message) {
            statusText.text = "Ошибка: " + message
            loadingOverlay.visible = false
            console.error("Preview error:", message)
        }
    }

    Shortcut {
        sequence: "Ctrl+R"
        onActivated: {
            if (previewBridge && previewBridge.currentFile !== "") {
                previewBridge.reload()
            }
        }
    }

    Shortcut {
        sequence: "Ctrl+O"
        onActivated: {
            if (previewBridge) {
                previewBridge.openFileDialog()
            }
        }
    }

    Component.onCompleted: {
        if (previewBridge) {
            previewBridge.refreshSamples()
        }
    }
}
