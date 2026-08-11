import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0
import "../styles"

Rectangle {
    id: root
    color: Colors.background
    
    property string title: "Вкладка"
    property string endpoint: ""
    property string method: "GET"
    property string accessToken: ""
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 10
        
        RowLayout {
            Text {
                text: root.title
                font.pixelSize: 18
                font.bold: true
                color: Colors.text
                Layout.fillWidth: true
            }
            
            Button {
                text: "Обновить"
                onClicked: loadData()
            }
        }
        
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            
            TextArea {
                id: textArea
                readOnly: true
                color: Colors.text
                font.pixelSize: 12
                font.family: "Monospace"
                background: Rectangle {
                    color: Colors.surface
                    border.color: Colors.border
                    radius: 6
                }
                padding: 10
            }
        }
    }
    
    function loadData() {
        var request = new XMLHttpRequest()
        request.open(method, endpoint, true)
        request.setRequestHeader("Authorization", "Bearer " + accessToken)
        request.setRequestHeader("Content-Type", "application/json")
        
        request.onreadystatechange = function() {
            if (request.readyState === XMLHttpRequest.DONE) {
                if (request.status === 200) {
                    var response = JSON.parse(request.responseText)
                    textArea.text = JSON.stringify(response, null, 2)
                } else {
                    textArea.text = "Ошибка " + request.status + ": " + request.statusText
                }
            }
        }
        request.send()
    }
    
    Component.onCompleted: loadData()
}
