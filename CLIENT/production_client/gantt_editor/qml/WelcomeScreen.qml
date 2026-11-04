import QtQuick 6.0
import QtQuick.Controls 6.0

Rectangle
{
    id: root
    color: "#f5f5f5"
    signal newProjectRequested()
    signal openProjectRequested()
    
    Column
    {
        anchors.centerIn: parent
        spacing: 30
        width: 400
        
        Rectangle
        {
            width: parent.width
            height: 200
            color: "white"
            radius: 8
            border.color: "#cccccc"
            Column
            {
                anchors.fill: parent
                anchors.margins: 15
                spacing: 10
                Text { text: "Горячие клавиши"; font.pixelSize: 18; font.bold: true }
                Row
                {
                    spacing: 20
                    Column
                    {
                        spacing: 5
                        Text { text: "Ctrl+N"; font.bold: true }
                        Text { text: "Ctrl+O"; font.bold: true }
                        Text { text: "Ctrl+S"; font.bold: true }
                        Text { text: "Ctrl+Shift+S"; font.bold: true }
                        Text { text: "F1"; font.bold: true }
                    }
                    Column
                    {
                        spacing: 5
                        Text { text: "Новый график" }
                        Text { text: "Открыть график" }
                        Text { text: "Сохранить" }
                        Text { text: "Сохранить как" }
                        Text { text: "Полноэкранный режим" }
                    }
                }
            }
        }

        Button
        {
            width: parent.width; height: 50;
            text: "Создать новый график";
            font.pixelSize: 16;
            onClicked: root.newProjectRequested()
        }
        Button
        {
            width: parent.width;
            height: 50;
            text: "Открыть график";
            font.pixelSize: 16;
            onClicked: root.openProjectRequested()
        }
    }
}
