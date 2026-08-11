import QtQuick 6.0
import QtQuick.Controls 6.0

Rectangle {
    property var userData: ({})
    color: "transparent"
    
    Text {
        text: userData.full_name || "User"
        font.pixelSize: 16
    }
}
