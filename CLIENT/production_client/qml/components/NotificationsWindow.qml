import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0
import Styles 1.0

Popup {
    id: root
    width: 500
    height: 600
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    x: parent.width - width - 20
    y: 60
    
    property var notifications: []
    property var bridge: null
    
    background: Rectangle {
        color: Colors.surface
        radius: 8
        border.color: Colors.border
        border.width: 1
    }
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12
        
        RowLayout {
            Layout.fillWidth: true
            
            Text {
                text: "Уведомления"
                font.pixelSize: 18
                font.bold: true
                color: Colors.text
                Layout.fillWidth: true
            }
            
            Button {
                text: "✕"
                flat: true
                implicitWidth: 30
                implicitHeight: 30
                font.pixelSize: 16
                onClicked: root.close()
            }
        }
        
        Rectangle {
            height: 1
            Layout.fillWidth: true
            color: Colors.border
        }
        
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            
            Column {
                width: parent.width
                spacing: 8
                padding: 4
                
                Repeater {
                    model: root.notifications
                    
                    Rectangle {
                        width: parent.width
                        radius: 6
                        color: modelData.status === "unread" ? Colors.primaryLight : Colors.surface
                        border.color: Colors.border
                        border.width: 1
                        
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 6
                            
                            RowLayout {
                                Layout.fillWidth: true
                                
                                Text {
                                    text: modelData.title || "Уведомление"
                                    font.pixelSize: 13
                                    font.bold: true
                                    color: modelData.status === "unread" ? Colors.primary : Colors.text
                                    Layout.fillWidth: true
                                    elide: Text.ElideRight
                                }
                                
                                Rectangle {
                                    visible: modelData.status === "unread"
                                    width: 8
                                    height: 8
                                    radius: 4
                                    color: Colors.primary
                                }
                                
                                Text {
                                    text: modelData.created_at ? new Date(modelData.created_at).toLocaleString() : ""
                                    font.pixelSize: 10
                                    color: Colors.textSecondary
                                }
                            }
                            
                            Rectangle {
                                visible: modelData.expanded || false
                                height: 1
                                Layout.fillWidth: true
                                color: Colors.border
                            }
                            
                            Text {
                                visible: modelData.expanded || false
                                text: modelData.message || "Нет текста"
                                font.pixelSize: 12
                                color: Colors.textSecondary
                                Layout.fillWidth: true
                                wrapMode: Text.WordWrap
                            }
                        }
                        
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: {
                                modelData.expanded = !modelData.expanded
                                if (modelData.status === "unread" && root.bridge) {
                                    root.bridge.markNotificationRead(modelData.notification_id)
                                }
                            }
                        }
                    }
                }
                
                Text {
                    visible: root.notifications.length === 0
                    text: "Нет уведомлений"
                    color: Colors.textSecondary
                    font.pixelSize: 14
                    anchors.horizontalCenter: parent.horizontalCenter
                    padding: 20
                }
            }
        }
        
        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            
            Button {
                text: "Отметить все прочитанными"
                flat: true
                visible: root.notifications.length > 0
                onClicked: {
                    if (root.bridge) {
                        root.bridge.markAllNotificationsRead()
                    }
                }
            }
            
            Item { Layout.fillWidth: true }
            
            Text {
                text: root.notifications.length + " уведомлений"
                color: Colors.textSecondary
                font.pixelSize: 11
            }
        }
    }
}
