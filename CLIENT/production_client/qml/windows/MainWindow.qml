import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0
import "../styles"
import "../dialogs" as Dialogs

Rectangle {
    id: root
    anchors.fill: parent
    color: Colors.background
    
    signal logoutRequested()
    signal themeToggleRequested()
    
    property var tiles: []
    
    // Верхняя панель
    Rectangle {
        id: topBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 85
        color: Colors.primary
        z: 1
        
        Rectangle {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            height: 1
            color: Colors.border
            opacity: 0.5
        }
        
        RowLayout {
            anchors.fill: parent
            anchors.topMargin: 8
            anchors.bottomMargin: 8
            anchors.leftMargin: 15
            anchors.rightMargin: 15
            spacing: 15
            
            ColumnLayout {
                spacing: 4
                Layout.fillWidth: true
                
                Text {
                    text: mainWindowBridge ? mainWindowBridge.userName : "Пользователь"
                    color: Colors.text
                    font.pixelSize: 17
                    font.bold: true
                }
                Text {
                    text: mainWindowBridge ? mainWindowBridge.userPosition : ""
                    color: Colors.textSecondary
                    font.pixelSize: 14
                }
                Text {
                    text: mainWindowBridge ? mainWindowBridge.userEmail : ""
                    color: Colors.textSecondary
                    font.pixelSize: 13
                    opacity: 0.85
                }
            }
            
            ColumnLayout {
                spacing: 4
                Layout.alignment: Qt.AlignVCenter
                Layout.leftMargin: -20
                
                Text {
                    text: "Пароль действителен:"
                    color: Colors.textSecondary
                    font.pixelSize: 12
                }
                Text {
                    text: mainWindowBridge ? getPasswordText(mainWindowBridge.passwordDaysLeft) : "Загрузка..."
                    color: mainWindowBridge ? getPasswordColor(mainWindowBridge.passwordDaysLeft) : Colors.textSecondary
                    font.pixelSize: 18
                    font.bold: true
                }
            }
            
            RowLayout {
                spacing: 12
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                
                Button {
                    text: "🌓"
                    font.pixelSize: 22
                    flat: true
                    implicitWidth: 55
                    implicitHeight: 55
                    onClicked: root.themeToggleRequested()
                }
                
                Button {
                    text: "🔑"
                    font.pixelSize: 22
                    flat: true
                    implicitWidth: 55
                    implicitHeight: 55
                    onClicked: changePasswordDialog.open()
                }
                
                Button {
                    text: "🚪"
                    font.pixelSize: 22
                    flat: true
                    implicitWidth: 55
                    implicitHeight: 55
                    onClicked: {
                        mainWindowBridge.logout()
                        root.logoutRequested()
                    }
                }
            }
        }
    }
    
    // Рабочее пространство с вкладками
    Rectangle {
        anchors.top: topBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        color: Colors.background
        
        TabBar {
            id: tabBar
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 40
            background: Rectangle { color: Colors.surface }
        }
        
        StackLayout {
            id: stackLayout
            anchors.top: tabBar.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            currentIndex: tabBar.currentIndex
            
            // Главная вкладка с плитками
            Rectangle {
                id: homeTab
                color: Colors.background
                
                GridView {
                    id: tilesGrid
                    anchors.fill: parent
                    anchors.margins: 20
                    cellWidth: 200
                    cellHeight: 150
                    model: root.tiles
                    
                    delegate: Rectangle {
                        id: tileRect
                        width: 180
                        height: 120
                        radius: 12
                        color: Colors.surface
                        border.color: Colors.border
                        
                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 5
                            
                            Text {
                                text: modelData.label
                                color: Colors.text
                                font.pixelSize: 14
                                font.bold: true
                                horizontalAlignment: Text.AlignHCenter
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                            }
                            
                            // Пузырёк (badge)
                            Rectangle {
                                id: badge
                                visible: modelData.badge_count > 0
                                anchors.top: parent.top
                                anchors.right: parent.right
                                anchors.margins: -10
                                width: 28
                                height: 28
                                radius: 14
                                color: Colors.error
                                
                                Text {
                                    anchors.centerIn: parent
                                    text: modelData.badge_count
                                    color: Colors.buttonText
                                    font.pixelSize: 12
                                    font.bold: true
                                }
                            }
                        }
                        
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onEntered: parent.color = Colors.buttonHover
                            onExited: parent.color = Colors.surface
                            onClicked: {
                                openTab(modelData.label, modelData.endpoint, modelData.method)
                            }
                        }
                    }
                }
            }
        }
    }
    
    // Диалог смены пароля
    Dialogs.ChangePasswordDialog {
        id: changePasswordDialog
        onPasswordChanged: mainWindowBridge.checkPasswordExpiry()
    }
    
    // Функции
    function getPasswordColor(days) {
        if (days > 30) return Colors.success
        if (days >= 15) return Colors.warning
        return Colors.error
    }
    
    function getPasswordText(days) {
        if (days > 30) return "✓ " + days + " дн."
        if (days >= 15) return "⚠ " + days + " дн."
        return "❗ " + days + " дн."
    }
    
    function openTab(title, endpoint, method) {
        var tabComponent = Qt.createComponent("TabContent.qml")
        if (tabComponent.status === Component.Ready) {
            var tab = tabComponent.createObject(stackLayout, {
                title: title,
                endpoint: endpoint,
                method: method,
                accessToken: mainWindowBridge.accessToken
            })
            stackLayout.addItem(tab)
            tabBar.currentIndex = stackLayout.count - 1
        }
    }
    
    Component.onCompleted: {
        mainWindowBridge.loadTiles()
    }
    
    Connections {
        target: mainWindowBridge
        function onTilesLoaded(tilesList) {
            root.tiles = tilesList
        }
    }
}
