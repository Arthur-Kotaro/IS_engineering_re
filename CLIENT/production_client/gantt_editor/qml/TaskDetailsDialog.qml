import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0

Dialog
{
    id: root
    title: "Подробности задачи"
    width: 800
    height: 700
    modal: true
    standardButtons: Dialog.Close
    anchors.centerIn: Overlay.overlay

    property string taskId: ""
    property var taskData: ({})
    property var historyEvents: []

    function openForTask(tId) {
        root.taskId = tId
        root.taskData = (projectController && projectController.projectData)
                        ? projectController.projectData.taskModel.getTask(tId)
                        : ({})
        root.historyEvents = []
        root.open()
        if (projectController)
            projectController.loadTaskHistory(tId)
    }

    Connections
    {
        target: projectController
        function onTaskHistoryLoaded(tId, events) {
            if (tId === root.taskId) root.historyEvents = events
        }
    }

    ScrollView
    {
        anchors.fill: parent
        anchors.margins: 15
        clip: true

        ColumnLayout
        {
            width: parent.width
            spacing: 8

            Label { text: "Название:"; font.bold: true }
            Label { text: root.taskData.title || "—"; wrapMode: Text.WordWrap; Layout.fillWidth: true }

            Label { text: "Ответственный:"; font.bold: true }
            Label { text: root.taskData.responsible || "—" }

            Label { text: "Группа:"; font.bold: true }
            Label { text: root.taskData.groupId || "—" }

            Label { text: "Статус:"; font.bold: true }
            Label { text: {
                var s = root.taskData.status
                if (s === 0) return "Запланировано"
                if (s === 1) return "Выполнено"
                if (s === 2) return "Имеются риски"
                if (s === 3) return "Блокировано"
                return "—"
            } }

            Label { text: "Целевые сроки:"; font.bold: true }
            Label { text: {
                var s = root.taskData.startDate
                var e = root.taskData.endDate
                if (!s || !e) return "—"
                return Qt.formatDateTime(new Date(s), "dd.MM.yyyy") + " — " +
                       Qt.formatDateTime(new Date(e), "dd.MM.yyyy")
            } }

            Label { text: "Прогнозные сроки:"; font.bold: true }
            Label { text: {
                var s = root.taskData.forecastStart
                var e = root.taskData.forecastEnd
                if (!s || !e) return "—"
                return Qt.formatDateTime(new Date(s), "dd.MM.yyyy") + " — " +
                       Qt.formatDateTime(new Date(e), "dd.MM.yyyy")
            } }

            Label { text: "Прогресс:"; font.bold: true }
            Label { text: {
                var c = root.taskData.progressCurrent
                var t = root.taskData.progressTotal
                if (t > 0 && c >= 0) return c + " / " + t
                return "—"
            } }

            Label { text: "Локальный график:"; font.bold: true }
            Label { text: {
                var l = root.taskData.localGraphState
                if (l === 1) return "требуется"
                if (l === 2) return "привязан"
                if (l === 3) return "отсутствует"
                return "не требуется"
            } }

            Label { text: "Комментарий:"; font.bold: true }
            Label { text: root.taskData.comment || "—"; wrapMode: Text.WordWrap; Layout.fillWidth: true }

            Rectangle { Layout.fillWidth: true; height: 1; color: "#cccccc" }

            Label { text: "История изменений:"; font.bold: true; font.pixelSize: 14 }

            Repeater
            {
                model: root.historyEvents
                delegate: Rectangle
                {
                    Layout.fillWidth: true
                    height: col.height + 12
                    color: "#f5f5f5"
                    radius: 4
                    border.color: "#dddddd"

                    ColumnLayout
                    {
                        id: col
                        anchors.fill: parent
                        anchors.margins: 6
                        spacing: 2

                        Label { text: "Дата: " + (modelData.created_at || "—"); font.pixelSize: 11 }
                        Label { text: "Событие: " + (modelData.event_type || "—"); font.pixelSize: 11 }
                        Label { text: "Поле: " + (modelData.field || "—"); font.pixelSize: 11 }
                        Label {
                            text: "Было: " + JSON.stringify(modelData.old_value) +
                                  " → Стало: " + JSON.stringify(modelData.new_value)
                            font.pixelSize: 11; wrapMode: Text.WordWrap; Layout.fillWidth: true
                        }
                        Label {
                            visible: modelData.reason && modelData.reason.length > 0
                            text: "Причина: " + (modelData.reason || "")
                            font.pixelSize: 11; color: "#c62828"; wrapMode: Text.WordWrap; Layout.fillWidth: true
                        }
                        Label { text: "Автор (id): " + (modelData.actor_id || "—"); font.pixelSize: 11 }
                    }
                }
            }

            Label {
                visible: root.historyEvents.length === 0
                text: "История пуста"
                color: "#888"
                Layout.fillWidth: true
            }
        }
    }
}
