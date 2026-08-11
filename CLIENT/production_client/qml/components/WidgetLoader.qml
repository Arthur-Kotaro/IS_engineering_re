import QtQuick 6.0
import QtQuick.Controls 6.0
import QtQuick.Layouts 6.0

ColumnLayout {
    id: root
    property string userRole: "user"
    property string widgetsPath: "widgets/common/"
    spacing: 20
    
    // Список общих виджетов (для всех пользователей)
    property var commonWidgets: [
        { name: "DashboardWidget", title: "Дашборд" },
        { name: "NotificationsWidget", title: "Уведомления" },
        { name: "ProjectsWidget", title: "Мои проекты" },
        { name: "CalendarWidget", title: "Календарь" }
    ]
    
    // Компонент для загрузки виджета
    Component {
        id: widgetComponent
        Rectangle {
            width: parent.width
            radius: 12
            color: Colors.surface
            border.color: Colors.border
            border.width: 1
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 15
                
                // Заголовок виджета
                RowLayout {
                    Layout.fillWidth: true
                    
                    Text {
                        text: widgetTitle
                        font.pixelSize: Typography.h4
                        font.bold: true
                        color: Colors.text
                        Layout.fillWidth: true
                    }
                }
                
                Rectangle {
                    height: 1
                    Layout.fillWidth: true
                    color: Colors.border
                }
                
                // Загрузчик содержимого
                Loader {
                    id: contentLoader
                    Layout.fillWidth: true
                    Layout.minimumHeight: 200
                    source: root.widgetsPath + widgetSource
                    
                    onLoaded: {
                        if (contentLoader.item) {
                            contentLoader.item.userRole = root.userRole
                        }
                    }
                }
            }
            
            property string widgetTitle: ""
            property string widgetSource: ""
        }
    }
    
    // Функция загрузки виджетов
    function loadWidgets() {
        // Очищаем текущие виджеты
        while (children.length > 0) {
            var child = children[0]
            if (child !== widgetLoader && child !== busyIndicator) {
                child.destroy()
            }
        }
        
        // Загружаем общие виджеты
        for (var i = 0; i < commonWidgets.length; i++) {
            var widget = commonWidgets[i]
            var widgetObj = widgetComponent.createObject(root, {
                "widgetTitle": widget.title,
                "widgetSource": widget.name + ".qml"
            })
            if (widgetObj) {
                widgetObj.Layout.fillWidth = true
            }
        }
    }
    
    // Загружаем виджеты при изменении пути
    onWidgetsPathChanged: loadWidgets()
    
    Component.onCompleted: loadWidgets()
}
