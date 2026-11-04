import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0

Dialog {
    id: root
    title: "О программе"
    width: 500
    height: 400
    modal: true
    standardButtons: Dialog.Ok
    anchors.centerIn: Overlay.overlay

    Column {
        anchors.fill: parent
        anchors.margins: 25
        spacing: 20

        Text {
            text: "Мастерграфик :re"
            font.pixelSize: 28
            font.bold: true
            anchors.horizontalCenter: parent.horizontalCenter
        }
        
        Text {
            text: "Версия 1.0.0"
            font.pixelSize: 14
            anchors.horizontalCenter: parent.horizontalCenter
        }
        
        Rectangle { height: 1; Layout.fillWidth: true; color: "#cccccc"; width: parent.width }
        
        Item { height: 15; width: parent.width }
        
        Text {
            text: "О разработчике:"
            font.pixelSize: 16
            font.bold: true
        }
        
        Text {
            text: "Артур Котаро"
            font.pixelSize: 14
        }
        
        Text {
            text: "arthur.kotaro@yandex.ru"
            font.pixelSize: 14
        }
        
        Item { height: 10; width: parent.width }
        
        Text {
            text: "© 2025 Мастерграфик"
            font.pixelSize: 12
            color: "gray"
            anchors.horizontalCenter: parent.horizontalCenter
        }
    }
}
