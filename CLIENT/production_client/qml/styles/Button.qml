import QtQuick 6.0
import QtQuick.Controls 6.0
import "../styles"

Button {
    id: control
    implicitHeight: 40
    implicitWidth: Math.max(80, implicitContentWidth + 30)
    
    background: Rectangle {
        color: control.enabled ?
               (control.down ? Colors.buttonPressed :
                control.hovered ? Colors.buttonHover : Colors.button) :
               "#424242"
        radius: 6
    }
    
    contentItem: Text {
        text: control.text
        color: Colors.buttonText
        font.pixelSize: 14
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
}
