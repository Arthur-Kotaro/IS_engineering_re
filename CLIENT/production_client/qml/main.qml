import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Window 6.0
import Styles 1.0
import "windows" as Windows

ApplicationWindow {
    id: root
    width: 450
    height: 580
    visible: true
    title: "Engineering :re"

    minimumWidth: 450
    maximumWidth: 450
    minimumHeight: 580
    maximumHeight: 580

    flags: Qt.Window | Qt.WindowCloseButtonHint | Qt.WindowMinimizeButtonHint

    property bool isFullScreen: false

    function applyThemeFromConfig() {
        if (!configManager) return
        
        if (configManager.theme === "dark") {
            Colors.isDarkTheme = true
            var darkIdx = configManager.darkScheme === "blue" ? 0 : configManager.darkScheme === "green" ? 1 : 2
            Colors.darkScheme = darkIdx
        } else {
            Colors.isDarkTheme = false
            var lightIdx = configManager.lightScheme === "blue" ? 0 : configManager.lightScheme === "green" ? 1 : 2
            Colors.lightScheme = lightIdx
        }
        Colors.updateColors()
    }

    function toggleTheme() {
        if (Colors.isDarkTheme) {
            // Переключаем на светлую тему с сохранённой схемой
            Colors.isDarkTheme = false
            var lightIdx = configManager.lightScheme === "blue" ? 0 : configManager.lightScheme === "green" ? 1 : 2
            Colors.lightScheme = lightIdx
            configManager.setTheme("light")
        } else {
            // Переключаем на тёмную тему с сохранённой схемой
            Colors.isDarkTheme = true
            var darkIdx = configManager.darkScheme === "blue" ? 0 : configManager.darkScheme === "green" ? 1 : 2
            Colors.darkScheme = darkIdx
            configManager.setTheme("dark")
        }
        Colors.updateColors()
    }

    function toggleFullScreen() {
        if (root.isFullScreen) {
            showNormal()
            root.isFullScreen = false
        } else {
            showFullScreen()
            root.isFullScreen = true
        }
    }

    Component.onCompleted: {
        applyThemeFromConfig()
    }

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: authWindowComponent
    }

    Component {
        id: authWindowComponent
        Windows.AuthWindow {
            Connections {
                target: authBridge
                function onLoginSuccess() {
                    root.minimumWidth = 800
                    root.maximumWidth = 16384
                    root.minimumHeight = 600
                    root.maximumHeight = 16384
                    root.width = 1024
                    root.height = 768
                    stackView.replace(mainWindowComponent)
                }
                function onLoginFailed(message) {
                    // Обработка ошибки уже в AuthWindow
                }
            }
        }
    }

    Component {
        id: mainWindowComponent
        Windows.MainWindow {
            onLogoutRequested: {
                root.minimumWidth = 450
                root.maximumWidth = 450
                root.minimumHeight = 580
                root.maximumHeight = 580
                root.width = 450
                root.height = 580
                stackView.replace(authWindowComponent)
            }
            onThemeToggleRequested: {
                root.toggleTheme()
            }
            onFullScreenToggled: {
                root.toggleFullScreen()
            }
        }
    }
}
