import QtQuick 6.0

Rectangle {
    property var userData: ({})
    property bool offlineMode: false
    color: Colors.background
    
    Text {
        anchors.centerIn: parent
        text: "DashboardView"
        font.pixelSize: 24
    }
}
