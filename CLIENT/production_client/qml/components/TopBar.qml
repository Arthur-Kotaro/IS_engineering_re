import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0
import Styles 1.0

Rectangle {
    id: root
    property string userName: ""
    property string userEmail: ""
    property string userPosition: ""
    property int passwordDaysLeft: 0
    property bool passwordExpired: false
    
    signal logoutRequested()
    signal passwordChangeRequested()
    signal fullScreenToggled()
    
    color: Colors.primary
    radius: 0
    
    RowLayout {
        anchors.fill: parent
        anchors.margins: 10
        spacing: 15
        
        Image {
            source: "qrc:/assets/images/icons/logo.svg"
            width: 40
            height: 40
            fillMode: Image.PreserveAspectFit
        }
        
        Text {
            text: "Корпоративная ИС"
            font.pixelSize: 18
            font.bold: true
            color: Colors.onPrimary
            Layout.fillWidth: true
        }
        
        Rectangle {
            visible: !passwordExpired && passwordDaysLeft > 0 && passwordDaysLeft <= 7
            height: 40
            width: warningText.implicitWidth + 20
            radius: 20
            color: Colors.warning
            
            RowLayout {
                anchors.centerIn: parent
                spacing: 8
                
                Text {
                    id: warningText
                    text: "⚠ Пароль истекает через " + passwordDaysLeft + " дн."
                    font.pixelSize: 12
                    color: Colors.onWarning
                }
            }
        }
        
        Rectangle {
            visible: passwordExpired
            height: 40
            width: expiredText.implicitWidth + 20
            radius: 20
            color: Colors.error
            
            RowLayout {
                anchors.centerIn: parent
                spacing: 8
                
                Text {
                    id: expiredText
                    text: "❌ Пароль просрочен!"
                    font.pixelSize: 12
                    color: Colors.onError
                }
            }
        }
        
        Rectangle {
            width: 1
            height: 40
            color: Colors.divider
            visible: userName !== ""
        }
        
        ColumnLayout {
            visible: userName !== ""
            spacing: 2
            
            Text {
                text: userName
                font.pixelSize: 14
                font.bold: true
                color: Colors.onPrimary
            }
            
            Text {
                text: userPosition
                font.pixelSize: 11
                color: Colors.onPrimaryVariant
            }
            
            Text {
                text: userEmail
                font.pixelSize: 10
                color: Colors.onPrimaryVariant
                opacity: 0.8
            }
        }
        
        Button {
            text: "Сменить пароль"
            icon.source: "qrc:/assets/images/icons/password.svg"
            onClicked: root.passwordChangeRequested()
            background: Rectangle {
                color: parent.down ? Colors.primaryDark : Colors.primaryLight
                radius: 20
            }
        }
        
        Button {
            icon.source: "qrc:/assets/images/icons/fullscreen.svg"
            onClicked: root.fullScreenToggled()
            flat: true
        }
        
        Button {
            text: "Выйти"
            icon.source: "qrc:/assets/images/icons/logout.svg"
            onClicked: root.logoutRequested()
            flat: true
        }
    }
}
