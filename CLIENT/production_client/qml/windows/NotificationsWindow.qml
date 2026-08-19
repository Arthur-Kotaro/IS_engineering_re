import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0
import Styles 1.0

Window
{
    id: root
    title: "Уведомления"
    width: 1200
    height: 600
    modality: Qt.WindowModal
    flags: Qt.Dialog | Qt.WindowCloseButtonHint

    property var notifications: []
    property int unreadCount: 0
    property bool loading: false

    ColumnLayout
    {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 10

        RowLayout
        {
            Layout.fillWidth: true
            spacing: 10

            Text {
                text: "Уведомления"
                font.pixelSize: 22
                font.bold: true
                color: Colors.text
                Layout.fillWidth: true
            }

            Text {
                text: "Непрочитанных: " + root.unreadCount
                font.pixelSize: 16
                color: Colors.textSecondary
                visible: root.unreadCount > 0
            }

            Button {
                text: "Прочитать все"
                enabled: root.unreadCount > 0
                onClicked: markAllAsRead()
                flat: true
                font.pixelSize: 16
            }

            Button {
                text: "Обновить"
                onClicked: loadNotifications()
                flat: true
                font.pixelSize: 16
            }
        }

        Rectangle
        {
            height: 1
            Layout.fillWidth: true
            color: Colors.border
        }

        ScrollView
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            visible: !root.loading

            ListView
            {
                id: listView
                anchors.fill: parent
                model: root.notifications
                spacing: 8

                delegate: Rectangle
                {
                    id: delegateRoot
                    width: listView.width
                    height: expanded ? contentHeight + 80 : 40
                    radius: 8
                    color: modelData.status === "unread" ? Colors.primary : Colors.surface
                    border.color: Colors.border
                    border.width: 1

                    property bool expanded: false
                    property int contentHeight: 0

                    ColumnLayout
                    {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 5

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 8

                            Rectangle {
                                width: 8
                                height: 8
                                radius: 4
                                color: modelData.status === "unread" ? Colors.error : "transparent"
                                visible: modelData.status === "unread"
                            }

                            Text {
                                text: modelData.title || "Уведомление"
                                font.pixelSize: 14
                                font.bold: modelData.status === "unread"
                                color: Colors.text
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }

                            Text {
                                text: modelData.created_at ? formatDate(modelData.created_at) : ""
                                font.pixelSize: 14
                                color: Colors.textSecondary
                            }
                        }

                        Text {
                            id: messageText
                            text: modelData.message || ""
                            font.pixelSize: 16
                            color: Colors.textSecondary
                            Layout.fillWidth: true
                            wrapMode: Text.WordWrap
                            visible: delegateRoot.expanded
                            onHeightChanged: {
                                delegateRoot.contentHeight = height
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 10
                            visible: delegateRoot.expanded

                            Button {
                                text: "Пометить прочитанным"
                                onClicked: markAsRead(modelData.notification_id)
                                flat: true
                                font.pixelSize: 14
                            }

                            Item {
                                Layout.fillWidth: true
                            }
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            delegateRoot.expanded = !delegateRoot.expanded
                            if (delegateRoot.expanded && modelData.status === "unread") {
                                markAsRead(modelData.notification_id)
                            }
                        }
                    }
                }
            }
        }

        BusyIndicator {
            running: root.loading
            visible: root.loading
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Text {
            text: "Нет уведомлений"
            font.pixelSize: 14
            color: Colors.textSecondary
            Layout.alignment: Qt.AlignHCenter
            visible: !root.loading && root.notifications.length === 0
        }
    }

    function formatDate(dateString) {
        var date = new Date(dateString)
        return date.toLocaleString("ru-RU", {
            day: "2-digit",
            month: "2-digit",
            year: "numeric",
            hour: "2-digit",
            minute: "2-digit"
        })
    }

    function loadNotifications() {
        root.loading = true
        var request = new XMLHttpRequest()
        request.open("GET", "http://localhost:8080/api/v1/notifications?limit=50", true)
        request.setRequestHeader("Authorization", "Bearer " + mainWindowBridge.accessToken)
        request.setRequestHeader("Content-Type", "application/json")

        request.onreadystatechange = function() {
            if (request.readyState === XMLHttpRequest.DONE) {
                root.loading = false
                if (request.status === 200) {
                    try {
                        var response = JSON.parse(request.responseText)
                        root.notifications = response.notifications || []
                        root.unreadCount = response.unread_count || 0
                        console.log("Notifications loaded:", root.notifications.length)
                    } catch (e) {
                        console.log("Parse error:", e)
                    }
                } else {
                    console.log("Load notifications error:", request.status)
                }
            }
        }
        request.send()
    }

    function loadUnreadCount() {
        var request = new XMLHttpRequest()
        request.open("GET", "http://localhost:8080/api/v1/notifications/unread/count", true)
        request.setRequestHeader("Authorization", "Bearer " + mainWindowBridge.accessToken)

        request.onreadystatechange = function() {
            if (request.readyState === XMLHttpRequest.DONE) {
                if (request.status === 200) {
                    try {
                        var response = JSON.parse(request.responseText)
                        root.unreadCount = response.unread_count || 0
                        topBar.unreadNotifications = root.unreadCount
                    } catch (e) {
                        console.log("Parse error:", e)
                    }
                }
            }
        }
        request.send()
    }

    function markAsRead(notificationId) {
        var request = new XMLHttpRequest()
        request.open("POST", "http://localhost:8080/api/v1/notifications/read", true)
        request.setRequestHeader("Authorization", "Bearer " + mainWindowBridge.accessToken)
        request.setRequestHeader("Content-Type", "application/json")

        var data = JSON.stringify({ "notification_ids": [notificationId] })

        request.onreadystatechange = function() {
            if (request.readyState === XMLHttpRequest.DONE) {
                if (request.status === 200) {
                    loadNotifications()
                    loadUnreadCount()
                }
            }
        }
        request.send(data)
    }

    function markAllAsRead() {
        var request = new XMLHttpRequest()
        request.open("POST", "http://localhost:8080/api/v1/notifications/read/all", true)
        request.setRequestHeader("Authorization", "Bearer " + mainWindowBridge.accessToken)
        request.setRequestHeader("Content-Type", "application/json")

        request.onreadystatechange = function() {
            if (request.readyState === XMLHttpRequest.DONE) {
                if (request.status === 200) {
                    loadNotifications()
                    loadUnreadCount()
                }
            }
        }
        request.send()
    }

    onVisibleChanged: {
        if (visible) {
            loadNotifications()
            loadUnreadCount()
        }
    }
}
