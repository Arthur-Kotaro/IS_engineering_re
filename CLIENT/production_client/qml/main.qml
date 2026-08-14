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

    Component.onCompleted: {
        Colors.setDarkTheme()
    }

    function toggleTheme() {
        if (Colors.isDarkTheme) {
            Colors.setLightTheme()
        } else {
            Colors.setDarkTheme()
        }
    }

    StackView {
        id: stackView
        anchors.fill: parent
        initialItem: authWindowComponent
    }

    Component {
        id: authWindowComponent
        Windows.AuthWindow {
            onLoginSuccess: {
                root.minimumWidth = 800
                root.maximumWidth = 16384
                root.minimumHeight = 600
                root.maximumHeight = 16384
                root.width = 1024
                root.height = 768
                stackView.replace(mainWindowComponent)
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
        }
    }
}
