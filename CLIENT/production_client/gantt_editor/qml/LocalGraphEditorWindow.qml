import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Window 6.0
import GanttEditor 1.0

ApplicationWindow
{
    id: lgWindow
    width: 1200
    height: 600
    visible: false
    title: "Локальный график"

    property string taskId: ""
    property string busUrl: ""

    function loadFromBus() {
        if (!busUrl || !taskId) return
        console.log("LocalGraphEditor: load from", busUrl, "task", taskId)

        var xhr = new XMLHttpRequest()
        xhr.open("GET", busUrl + "/internal/local-graphs/by-task/" + taskId)
        xhr.onreadystatechange = function() {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                if (xhr.status === 200) {
                    var data = JSON.parse(xhr.responseText)
                    console.log("LocalGraphEditor: loaded", data)
                    title = "Локальный график: " + (data.title || "")
                    lgIdLabel.text = "LG ID: " + (data.lg_id || "—")
                    targetStartLabel.text = "Цель: " + (data.target_start || "—")
                    targetEndLabel.text = " — " + (data.target_end || "—")
                    responsibleLabel.text = "Ответственный ID: " + (data.responsible_id || "—")
                } else {
                    lgIdLabel.text = "Ошибка: " + xhr.status
                }
            }
        }
        xhr.send()
    }

    onVisibleChanged: { if (visible) loadFromBus() }

    Rectangle {
        anchors.fill: parent
        color: "#f8f8f8"

        Column {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 10

            Label { id: lgIdLabel; text: "Загрузка..."; font.pixelSize: 14; font.bold: true }
            Label { id: targetStartLabel; text: ""; font.pixelSize: 13 }
            Label { id: targetEndLabel; text: ""; font.pixelSize: 13 }
            Label { id: responsibleLabel; text: ""; font.pixelSize: 13 }

            Rectangle {
                width: parent.width
                height: 2
                color: "#cccccc"
            }

            Label {
                text: "(TODO: редактор ЛГ — задачи, зависимости, статусы)"
                font.pixelSize: 12
                color: "#666666"
            }
        }
    }
}
