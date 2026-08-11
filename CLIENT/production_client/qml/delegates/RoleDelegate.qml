import QtQuick 6.0

Item {
    property var modelData: ({})
    height: 40
    
    Text {
        text: modelData.name || modelData.role || ""
        anchors.verticalCenter: parent.verticalCenter
    }
}
